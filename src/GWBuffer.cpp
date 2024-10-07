#include "GWBuffer.hpp"

// std
#include <cassert>
#include <cstring>

namespace GWIN
{

  VkDeviceSize GWBuffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
  {
    if (minOffsetAlignment > 0)
    {
      return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }
    return instanceSize;
  }

  GWBuffer::GWBuffer(
      GWinDevice &device,
      VkDeviceSize instanceSize,
      uint32_t instanceCount,
      VkBufferUsageFlags usageFlags,
      VmaMemoryUsage memoryUsage,
      VkDeviceSize minOffsetAlignment)
      : device{device},
        instanceSize{instanceSize},
        instanceCount{instanceCount},
        usageFlags{usageFlags},
        memoryUsage{memoryUsage}
  {
    alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
    bufferSize = alignmentSize * instanceCount;
    device.createBuffer(bufferSize, usageFlags, memoryUsage, buffer, bufferAllocation);

    if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
      VkBufferDeviceAddressInfo bufferDeviceAddressInfo = {};
      bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
      bufferDeviceAddressInfo.buffer = buffer;

      bufferAddress = static_cast<DeviceAddress>(vkGetBufferDeviceAddress(device.device(), &bufferDeviceAddressInfo));
    }
  }

  GWBuffer::~GWBuffer()
  {
    unmap();
    vmaDestroyBuffer(device.getAllocator(), buffer, bufferAllocation);
  }

  VkResult GWBuffer::map(VkDeviceSize size, VkDeviceSize offset)
  {
    assert(buffer && bufferAllocation && "Called map on buffer before create");
    return vmaMapMemory(device.getAllocator(), bufferAllocation, &mapped);
  }

  void GWBuffer::unmap()
  {
    if (mapped)
    {
      vmaUnmapMemory(device.getAllocator(), bufferAllocation);
      mapped = nullptr;
    }
  }

  void GWBuffer::writeToBuffer(void *data, VkDeviceSize size, VkDeviceSize offset)
  {
    assert(mapped && "Cannot copy to unmapped buffer");

    if (size == VK_WHOLE_SIZE)
    {
      memcpy(mapped, data, bufferSize);
    }
    else
    {
      char *memOffset = (char *)mapped;
      memOffset += offset;
      memcpy(memOffset, data, size);
    }
  }

  VkResult GWBuffer::flush(VkDeviceSize size, VkDeviceSize offset)
  {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = device.getBufferMemory(bufferAllocation);
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vmaFlushAllocation(device.getAllocator(), bufferAllocation, offset, size);
  }

  VkResult GWBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
  {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = device.getBufferMemory(bufferAllocation);
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vmaInvalidateAllocation(device.getAllocator(), bufferAllocation, offset, size);
  }

  VkDescriptorBufferInfo GWBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset)
  {
    return VkDescriptorBufferInfo{
        buffer,
        offset,
        size,
    };
  }

  void GWBuffer::writeToIndex(void *data, int index)
  {
    writeToBuffer(data, instanceSize, index * alignmentSize);
  }

  VkResult GWBuffer::flushIndex(int index) { return flush(alignmentSize, index * alignmentSize); }

  VkDescriptorBufferInfo GWBuffer::descriptorInfoForIndex(int index)
  {
    return descriptorInfo(alignmentSize, index * alignmentSize);
  }

  VkResult GWBuffer::invalidateIndex(int index)
  {
    return invalidate(alignmentSize, index * alignmentSize);
  }
} // namespace GWIN
