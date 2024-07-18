#pragma once
 
#include "GWDevice.hpp"
#include "vma/vk_mem_alloc.h"

namespace GWIN {
 
class GWBuffer {
 public:
   GWBuffer(
       GWinDevice &device,
       VkDeviceSize instanceSize,
       uint32_t instanceCount,
       VkBufferUsageFlags usageFlags,
       VmaMemoryUsage memoryUsage,
       VkDeviceSize minOffsetAlignment = 1);
   ~GWBuffer();

   GWBuffer(const GWBuffer &) = delete;
   GWBuffer &operator=(const GWBuffer &) = delete;

   VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
   void unmap();

   void writeToBuffer(void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
   VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
   VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
   VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

   void writeToIndex(void *data, int index);
   VkResult flushIndex(int index);
   VkDescriptorBufferInfo descriptorInfoForIndex(int index);
   VkResult invalidateIndex(int index);

   VkBuffer getBuffer() const { return buffer; }
   void *getMappedMemory() const { return mapped; }
   uint32_t getInstanceCount() const { return instanceCount; }
   VkDeviceSize getInstanceSize() const { return instanceSize; }
   VkDeviceSize getAlignmentSize() const { return instanceSize; }
   VkBufferUsageFlags getUsageFlags() const { return usageFlags; }
   VkDeviceSize getBufferSize() const { return bufferSize; }
 
 private:
   static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

   GWinDevice &device;
   void *mapped = nullptr;
   VkBuffer buffer = VK_NULL_HANDLE;
   VmaAllocation bufferAllocation = VK_NULL_HANDLE;

   VkDeviceSize bufferSize;
   uint32_t instanceCount;
   VkDeviceSize instanceSize;
   VkDeviceSize alignmentSize;
   VkBufferUsageFlags usageFlags;
   VmaMemoryUsage memoryUsage;
};
 
}  // namespace GWIN