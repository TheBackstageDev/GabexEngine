#include "GWDescriptors.hpp"
 
// std
#include <cassert>
#include <stdexcept>
#include <iostream>
 
namespace GWIN {
 
// *************** Descriptor Set Layout Builder *********************
 
GWDescriptorSetLayout::Builder &GWDescriptorSetLayout::Builder::addBinding(
    uint32_t binding,
    VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags,
    uint32_t count) {
  assert(bindings.count(binding) == 0 && "Binding already in use");
  VkDescriptorSetLayoutBinding layoutBinding{};
  layoutBinding.binding = binding;
  layoutBinding.descriptorType = descriptorType;
  layoutBinding.descriptorCount = count;
  layoutBinding.stageFlags = stageFlags;
  bindings[binding] = layoutBinding;
  return *this;
}
 
std::unique_ptr<GWDescriptorSetLayout> GWDescriptorSetLayout::Builder::build() const {
  return std::make_unique<GWDescriptorSetLayout>(device, bindings);
}
 
// *************** Descriptor Set Layout *********************
 
GWDescriptorSetLayout::GWDescriptorSetLayout(
    GWinDevice &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
    : device{device}, bindings{bindings} {
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
  for (auto kv : bindings) {
    setLayoutBindings.push_back(kv.second);
  }
 
  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
  descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
  descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
 
  if (vkCreateDescriptorSetLayout(
          device.device(),
          &descriptorSetLayoutInfo,
          nullptr,
          &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}
 
GWDescriptorSetLayout::~GWDescriptorSetLayout() {
  vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayout, nullptr);
}
 
// *************** Descriptor Pool Builder *********************
 
GWDescriptorPool::Builder &GWDescriptorPool::Builder::addPoolSize(
    VkDescriptorType descriptorType, uint32_t count) {
  poolSizes.push_back({descriptorType, count});
  return *this;
}
 
GWDescriptorPool::Builder &GWDescriptorPool::Builder::setPoolFlags(
    VkDescriptorPoolCreateFlags flags) {
  poolFlags = flags;
  return *this;
}
GWDescriptorPool::Builder &GWDescriptorPool::Builder::setMaxSets(uint32_t count) {
  maxSets = count;
  return *this;
}
 
std::unique_ptr<GWDescriptorPool> GWDescriptorPool::Builder::build() const {
  return std::make_unique<GWDescriptorPool>(device, maxSets, poolFlags, poolSizes);
}
 
// *************** Descriptor Pool *********************
 
GWDescriptorPool::GWDescriptorPool(
    GWinDevice &device,
    uint32_t maxSets,
    VkDescriptorPoolCreateFlags poolFlags,
    const std::vector<VkDescriptorPoolSize> &poolSizes)
    : device{device} {
  VkDescriptorPoolCreateInfo descriptorPoolInfo{};
  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  descriptorPoolInfo.pPoolSizes = poolSizes.data();
  descriptorPoolInfo.maxSets = maxSets;
  descriptorPoolInfo.flags = poolFlags;
 
  if (vkCreateDescriptorPool(device.device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}
 
GWDescriptorPool::~GWDescriptorPool() {
  vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);
}
 
bool GWDescriptorPool::allocateDescriptor(
    const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const {
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.pSetLayouts = &descriptorSetLayout;
  allocInfo.descriptorSetCount = 1;
 
  // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
  // a new pool whenever an old pool fills up.
  if (vkAllocateDescriptorSets(device.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
    return false;
  }
  return true;
}
 
void GWDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const {
  vkFreeDescriptorSets(
      device.device(),
      descriptorPool,
      static_cast<uint32_t>(descriptors.size()),
      descriptors.data());
}
 
void GWDescriptorPool::resetPool() {
  vkResetDescriptorPool(device.device(), descriptorPool, 0);
}
 
// *************** Descriptor Writer *********************
 
GWDescriptorWriter::GWDescriptorWriter(GWDescriptorSetLayout &setLayout, GWDescriptorPool &pool)
    : setLayout{setLayout}, pool{pool} {}
 
GWDescriptorWriter &GWDescriptorWriter::writeBuffer(
    uint32_t binding, VkDescriptorBufferInfo *bufferInfo) {
  assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
 
  auto &bindingDescription = setLayout.bindings[binding];
 
  assert(
      bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");
 
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pBufferInfo = bufferInfo;
  write.descriptorCount = 1;
 
  writes.push_back(write);
  return *this;
}
 
GWDescriptorWriter &GWDescriptorWriter::writeImage(
    uint32_t binding, VkDescriptorImageInfo *imageInfo, uint32_t dstArrayPos) {
    
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto &bindingDescription = setLayout.bindings[binding];
    
    assert(bindingDescription.descriptorCount > dstArrayPos && "Array position out of bounds");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstArrayElement = dstArrayPos; 
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;  

    writes.push_back(write);
    return *this;
}

bool GWDescriptorWriter::build(VkDescriptorSet &set, bool arraySet) {
  bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
  //If not a success, it means for the single set its a error, but for the array that it probably just
  //alreadly created a set for it to be
  if (!arraySet)
  {
    if (!success)
    {
      std::cout << "failed" << std::endl;
      return false;
    }
    overwrite(set);
  } else {
    overwrite(set);
  }

  return true;
}
 
void GWDescriptorWriter::overwrite(VkDescriptorSet &set) {
  for (auto &write : writes) {
    write.dstSet = set;
  }
  vkUpdateDescriptorSets(pool.device.device(), writes.size(), writes.data(), 0, nullptr);
}
 
}  // namespace GWIN