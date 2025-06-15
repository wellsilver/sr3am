#include <sr3am.h>

#include <vulkan/vulkan.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;

  VkPipelineLayout pipelinelayout;
  err = vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelinelayout);
  if (err != VK_SUCCESS) {
    printf("Err vkCreatePipelineLayout %i\n", err);

    return 1;
  }

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
  err = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &computepipeline);
  if (err != VK_SUCCESS) {
    printf("Err vkCreateComputePipelines %i\n", err);

    return 1;
  }

  VkCommandPoolCreateInfo commandpoolcreateinfo = {};
  commandpoolcreateinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandpoolcreateinfo.queueFamilyIndex = queueCreateInfo.queueFamilyIndex;

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

  err = vkBeginCommandBuffer(commandbuffer, &commandbufferbegininfo);
  if (err != VK_SUCCESS) {
    printf("Err vkBeginCommandBuffer %i\n", err);

    return 1;
  }

  vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computepipeline);
  vkCmdDispatch(commandbuffer, 12, 1, 1);

  err = vkEndCommandBuffer(commandbuffer);
  if (err != VK_SUCCESS) {
    printf("Err vkEndCommandBuffer %i\n", err);

    return 1;
  }

  VkSubmitInfo queuesubmitinfo = {};
  queuesubmitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  queuesubmitinfo.commandBufferCount = 1;
  queuesubmitinfo.pCommandBuffers = &commandbuffer;

  VkFenceCreateInfo fencecreateinfo = {};
  fencecreateinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  VkFence fence;
  vkCreateFence(device, &fencecreateinfo, NULL, &fence);
  
  err = vkQueueSubmit(devicequeue, 1, &queuesubmitinfo, fence);
  if (err != VK_SUCCESS) {
    printf("Err vkQueueSubmit %i\n", err);

    return 1;
  }

  vkWaitForFences(device, 1, &fence, VK_TRUE, 0);
  printf("Done\n");

  vkDeviceWaitIdle(device);
  vkDestroyFence(device, fence, NULL);
  vkDestroyCommandPool(device, commandpool, NULL);
  vkDestroyPipeline(device, computepipeline, NULL);
  vkDestroyPipelineLayout(device, pipelinelayout, NULL);
  vkDestroyShaderModule(device, shader, NULL);
  vkDestroyDevice(device, NULL);
  vkDestroyInstance(instance, NULL);
}