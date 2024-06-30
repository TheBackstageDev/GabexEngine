#pragma once
 
#include "../GWDevice.hpp"
 
// std
#include <memory>
#include <unordered_map>
#include <vector>
 
namespace GWIN {
 
class GWDescriptorSetLayout {
 public:
  class Builder {
   public:
    Builder(GWinDevice &device) : device{device} {}
 
    Builder &addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count = 1);
    std::unique_ptr<GWDescriptorSetLayout> build() const;
 
   private:
    GWinDevice &device;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
  };
 
  GWDescriptorSetLayout(
      GWinDevice &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
  ~GWDescriptorSetLayout();
  GWDescriptorSetLayout(const GWDescriptorSetLayout &) = delete;
  GWDescriptorSetLayout &operator=(const GWDescriptorSetLayout &) = delete;
 
  VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
 
 private:
  GWinDevice &device;
  VkDescriptorSetLayout descriptorSetLayout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
 
  friend class GWDescriptorWriter;
};
 
class GWDescriptorPool {
 public:
  class Builder {
   public:
    Builder(GWinDevice &device) : device{device} {}
 
    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<GWDescriptorPool> build() const;
 
   private:
    GWinDevice &device;
    std::vector<VkDescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    VkDescriptorPoolCreateFlags poolFlags = 0;
  };
 
  GWDescriptorPool(
      GWinDevice &device,
      uint32_t maxSets,
      VkDescriptorPoolCreateFlags poolFlags,
      const std::vector<VkDescriptorPoolSize> &poolSizes);
  ~GWDescriptorPool();
  GWDescriptorPool(const GWDescriptorPool &) = delete;
  GWDescriptorPool &operator=(const GWDescriptorPool &) = delete;
 
  bool allocateDescriptor(
      const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;
 
  void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;
 
  void resetPool();
  VkDescriptorPool getDescriptorPool() {return descriptorPool;}
 
 private:
  GWinDevice &device;
  VkDescriptorPool descriptorPool;
 
  friend class GWDescriptorWriter;
};
 
class GWDescriptorWriter {
 public:
  GWDescriptorWriter(GWDescriptorSetLayout &setLayout, GWDescriptorPool &pool);
 
  GWDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
  GWDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);
 
  bool build(VkDescriptorSet &set);
  void overwrite(VkDescriptorSet &set);
 
 private:
  GWDescriptorSetLayout &setLayout;
  GWDescriptorPool &pool;
  std::vector<VkWriteDescriptorSet> writes;
};
 
}  // namespace GWIN