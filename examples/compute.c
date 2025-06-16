#include <sr3am.h>

#include <vulkan/vulkan.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct rgba {unsigned char r,g,b,a;};

// Vulkan example renderer, using SR3AM and no instance or device extensions. (It draws to fb in normal ram using compute shader)

int main() {
  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

  VkApplicationInfo appInfo = {};
  createInfo.pApplicationInfo = &appInfo;
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.apiVersion = VK_API_VERSION_1_0;
  appInfo.applicationVersion = VK_MAKE_VERSION(0,0,0);
  appInfo.pApplicationName = "SR3AM compute";
  appInfo.engineVersion = VK_MAKE_VERSION(0,0,0);
  appInfo.pEngineName = "N/A";

  VkResult err;

  uint32_t layerlen = 1;
  char *layers[1] = {"VK_LAYER_KHRONOS_validation"};

  createInfo.enabledLayerCount = layerlen;
  createInfo.ppEnabledLayerNames = layers;

  VkInstance instance;
  err = vkCreateInstance(&createInfo, NULL, &instance);
  if (err != VK_SUCCESS) {
    printf("Error vkCreateInstance %i\n", err);
    
    return 1;
  }
  
  uint32_t deviceCount = 0;
  err = vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
  if (err != VK_SUCCESS) {
    printf("Error vkEnumeratePhysicalDevices %i\n");

    return -1;
  }
  if (deviceCount == 0) {
    printf("No devices\n");

    return 1;
  }

  VkPhysicalDevice devices[deviceCount];
  err = vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
  if (err != VK_SUCCESS) {
    printf("Error vkEnumeratePhysicalDevices %i\n");

    return -1;
  }

  // Ask the user to select a device
  printf("Select a device:\n", deviceCount);
  for (unsigned int loop=0;loop<deviceCount;loop++) {
    if (devices[loop] == 0) {printf("Broken device\n");break;}

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(devices[loop], &properties);

    printf("%i: %s\n", loop, properties.deviceName);
  }
  printf("> ");
  int selecteddevice = getchar()-48;

  // Create logical device from selected device
  VkDeviceCreateInfo devicecreateInfo = {};
  devicecreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  devicecreateInfo.queueCreateInfoCount = 1;

  // Create device queue
  VkDeviceQueueCreateInfo queueCreateInfo = {};
  devicecreateInfo.pQueueCreateInfos = &queueCreateInfo;
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueFamilyIndex = 9999; // 9999 is a error value
  queueCreateInfo.queueCount = 1;
  float queuePriority = 1.0f; // why is this separate from the struct?
  queueCreateInfo.pQueuePriorities = &queuePriority;

  // Find first queue family with compute
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(devices[selecteddevice], &queueFamilyCount, NULL);
  if (queueFamilyCount == 0) {
    printf("Err vkGetPhysicalDeviceQueueFamilyProperties (given pQueueFamilyPropertyCount is zero)\n");
    return 1;
  }

  VkQueueFamilyProperties queueFamilies[queueFamilyCount];
  vkGetPhysicalDeviceQueueFamilyProperties(devices[selecteddevice], &queueFamilyCount, queueFamilies);
  if (queueFamilies == NULL) {
    printf("Err vkGetPhysicalDeviceQueueFamilyProperties (given pQueueFamilyProperties is null, no queue families given)\n");
    return 1;
  }

  // Actually search for a queue family
  for (unsigned int loop=0;loop<queueFamilyCount;loop++)
    if (queueFamilies[loop].queueFlags & VK_QUEUE_COMPUTE_BIT) queueCreateInfo.queueFamilyIndex = loop;
  
  if (queueCreateInfo.queueFamilyIndex == 9999) {
    printf("No compute queue family\n");

    return 1;
  }
  
  VkDevice device;
  err = vkCreateDevice(devices[selecteddevice], &devicecreateInfo, NULL, &device);
  if (err != VK_SUCCESS) {
    printf("Err vkCreateDevice %i\n", err);
    
    return 1;
  }

  VkQueue devicequeue;
  vkGetDeviceQueue(device, queueCreateInfo.queueFamilyIndex, 0, &devicequeue);

  FILE *shadercodefd = fopen("compute.spv.o", "r");
  if (shadercodefd == 0) {
    printf("Couldnt open file compute.spv.o");
    return 1;
  }

  // read the shader to memory
  fseek(shadercodefd, 0, SEEK_END);
  unsigned int shadercodesize = ftell(shadercodefd);
  char shadercode[ftell(shadercodefd)];
  fseek(shadercodefd, 0, SEEK_SET);
  fread(shadercode, 1, shadercodesize, shadercodefd);

  VkShaderModuleCreateInfo shadercodecreate = {};
  shadercodecreate.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shadercodecreate.codeSize = shadercodesize;
  shadercodecreate.pCode = (uint32_t *) shadercode;
  shadercodecreate.flags = 0;

  VkShaderModule shader;
  err = vkCreateShaderModule(device, &shadercodecreate, NULL, &shader);
  if (err != VK_SUCCESS) {
    printf("Err vkCreateShaderModule %i\n", err);

    return 1;
  }

  VkCommandPoolCreateInfo commandpoolcreateinfo = {};
  commandpoolcreateinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandpoolcreateinfo.queueFamilyIndex = queueCreateInfo.queueFamilyIndex;
  commandpoolcreateinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  VkCommandPool commandpool;
  err = vkCreateCommandPool(device, &commandpoolcreateinfo, NULL, &commandpool);
  if (err != VK_SUCCESS) {
    printf("Err vkCreateCommandPool %i\n", err);

    return 1;
  }

  VkCommandBufferAllocateInfo commandbufferallocateinfo = {};
  commandbufferallocateinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  commandbufferallocateinfo.commandPool = commandpool;
  commandbufferallocateinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandbufferallocateinfo.commandBufferCount = 1;
  
  VkCommandBuffer commandbuffer;
  err = vkAllocateCommandBuffers(device, &commandbufferallocateinfo, &commandbuffer);
  if (err != VK_SUCCESS) {
    printf("Err vkAllocateCommandBuffers %i\n", err);

    return 1;
  }

  VkCommandBufferBeginInfo commandbufferbegininfo = {};
  commandbufferbegininfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  VkCommandBufferInheritanceInfo commandbufferinheritanceinfo = {};
  commandbufferbegininfo.pInheritanceInfo = &commandbufferinheritanceinfo;
  commandbufferbegininfo.flags = 0;

  VkSubmitInfo queuesubmitinfo = {};
  queuesubmitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  queuesubmitinfo.commandBufferCount = 1;
  queuesubmitinfo.pCommandBuffers = &commandbuffer;

  samImage window = samWindow("SR3AM Compute", 480, 480, -1, -1, 0);
  // returns NULL if failed
  if (window == NULL) {
    fwrite("Could not create window", 24, 1, stderr);
    return -1;
  }

  uint32_t width, height;
  uint64_t pos = 0; // camera position (X)

  while (!samClosing(window)) {
    struct rgba *px = samPixels(&width, &height, window);

    // From here on is to setup the image
    VkImageCreateInfo imagecreateinfo = {};
    imagecreateinfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imagecreateinfo.imageType = VK_IMAGE_TYPE_2D;
    imagecreateinfo.extent = (VkExtent3D) {.depth = 1, .height = height, .width = width};
    imagecreateinfo.mipLevels = 1;
    imagecreateinfo.arrayLayers = 1;
    imagecreateinfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imagecreateinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imagecreateinfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imagecreateinfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    imagecreateinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imagecreateinfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VkImage image;
    err = vkCreateImage(device, &imagecreateinfo, NULL, &image);
    if (err != VK_SUCCESS) {
      printf("Err vkCreateImage %i\n", err);

      return 1;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo memallocInfo = {};
    memallocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memallocInfo.allocationSize = memRequirements.size;
    memallocInfo.memoryTypeIndex = 0; // Sit and pray this works? lmao

    VkDeviceMemory imagememory;
    err = vkAllocateMemory(device, &memallocInfo, NULL, &imagememory);
    if (err != VK_SUCCESS) {
      printf("Err vkAllocateMemory %i\n", err);

      return 1;
    }

    vkBindImageMemory(device, image, imagememory, 0);

    VkImageViewCreateInfo viewCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_UNORM,
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
      },
    };

    VkImageView imageview;
    vkCreateImageView(device, &viewCreateInfo, NULL, &imageview);
    // Done setting up image

    VkDescriptorSetLayoutBinding imageBinding = {
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 1,
      .pBindings = &imageBinding,
    };

    VkDescriptorSetLayout descriptorSetLayout;
    err = vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &descriptorSetLayout);
    if (err != VK_SUCCESS) {
      printf("Err vkCreateDescriptorSetLayout %i\n", err);

      return 1;
    }

    VkPushConstantRange pushConstantRange = {
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
      .offset = 0,
      .size = sizeof(uint32_t),
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    VkPipelineLayout pipelinelayout;
    err = vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelinelayout);
    if (err != VK_SUCCESS) {
      printf("Err vkCreatePipelineLayout %i\n", err);

      return 1;
    }

    VkDescriptorPoolSize poolSize = {
      .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
      .descriptorCount = 1,
    };

    VkDescriptorPoolCreateInfo poolCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .poolSizeCount = 1,
      .pPoolSizes = &poolSize,
      .maxSets = 1,
    };

    VkDescriptorPool descriptorPool;
    vkCreateDescriptorPool(device, &poolCreateInfo, NULL, &descriptorPool);

    VkDescriptorSetAllocateInfo descallocInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = descriptorPool,
      .descriptorSetCount = 1,
      .pSetLayouts = &descriptorSetLayout,
    };

    VkDescriptorSet descriptorSet;
    vkAllocateDescriptorSets(device, &descallocInfo, &descriptorSet);

    VkDescriptorImageInfo imageInfo = {
      .imageView = imageview,
      .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
    };

    VkWriteDescriptorSet descriptorWrite = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = descriptorSet,
      .dstBinding = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
      .pImageInfo = &imageInfo,
    };

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, NULL);

    VkPipelineShaderStageCreateInfo shaderStageInfo = {};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageInfo.module = shader;
    shaderStageInfo.pName = "main";

    VkComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = shaderStageInfo;
    pipelineInfo.layout = pipelinelayout;

    VkPipeline computepipeline;
    err = vkCreateComputePipelines(device, NULL, 1, &pipelineInfo, NULL, &computepipeline);
    if (err != VK_SUCCESS) {
      printf("Err vkCreateComputePipelines %i\n", err);

      return 1;
    }

    err = vkBeginCommandBuffer(commandbuffer, &commandbufferbegininfo);
    if (err != VK_SUCCESS) {
      printf("Err vkBeginCommandBuffer %i\n", err);

      return 1;
    }

    vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computepipeline);
    
    vkCmdPushConstants(commandbuffer, pipelinelayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, 4, &pos);
    vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelinelayout, 0, 1, &descriptorSet, 0, NULL);
    vkCmdDispatch(commandbuffer, width, height, 1);
    

    err = vkEndCommandBuffer(commandbuffer);
    if (err != VK_SUCCESS) {
      printf("Err vkEndCommandBuffer %i\n", err);

      return 1;
    }

    err = vkQueueSubmit(devicequeue, 1, &queuesubmitinfo, NULL);
    if (err != VK_SUCCESS) {
      printf("Err vkQueueSubmit %i\n", err);

      return 1;
    }

    vkQueueWaitIdle(devicequeue);
    struct rgba *map;
    vkMapMemory(device, imagememory, 0, memRequirements.size, 0, (void **) &map);
    for (unsigned int loop=0;loop<width*height;loop++) {
      px[loop] = map[loop];
    }
    vkUnmapMemory(device, imagememory);
    vkResetCommandBuffer(commandbuffer, 0);
    vkDestroyImage(device, image, NULL);
    vkDestroyPipeline(device, computepipeline, NULL);
    vkDestroyPipelineLayout(device, pipelinelayout, NULL);
    vkDestroyDescriptorPool(device, descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
    vkFreeMemory(device, imagememory, NULL);
    samUpdate(window);
    samWait(window);
    pos++;
  }

  samClose(window);


  vkDeviceWaitIdle(device);
  vkDestroyCommandPool(device, commandpool, NULL);
  vkDestroyShaderModule(device, shader, NULL);
  vkDestroyDevice(device, NULL);
  vkDestroyInstance(instance, NULL);
}