#pragma once
#include"Graffiti/base/core.h"
#include<vector>
#include<vulkan/vulkan.h>
#include"Graffiti/base/Window.h"
struct GLFWwindow;
namespace Graffiti {
    
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    //��������
    //���ǵ��õĶ��е������ͷ���������
    struct QueueFamilyIndices {

        //����ֻ��Ҫ�������о����ˣ�һ��֧��ͼ����Ⱦ��һ��֧��ͼ�γ���
        uint32_t graphicsFamily;
        uint32_t presentFamily;
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };
    class VulkanDevice {
    public:
        VulkanDevice(std::unique_ptr<Window>& input_window);
        ~VulkanDevice();


        VkCommandPool getCommandPool() { return commandPool; }
        VkDevice device() { return device_; }
        VkSurfaceKHR surface() { return surface_; }
        VkQueue graphicsQueue() { return graphicsQueue_; }
        VkQueue presentQueue() { return presentQueue_; }
        VkPhysicalDevice  getphysicalDevice() { return physicalDevice; }
        VkInstance  getinstance() { return instance; }

        SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

        void createBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& bufferMemory);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        void createImageWithInfo(
            const VkImageCreateInfo& imageInfo,
            VkMemoryPropertyFlags properties,
            VkImage& image,
            VkDeviceMemory& imageMemory);
        void copyImage(VkBuffer srcBuffer, VkImage image, uint32_t width, uint32_t height);
        void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image,
            VkFormat format,
            VkImageLayout oldLayout, VkImageLayout newLayout);
        VkImageView createImageView(VkImage image, VkFormat format);
        VkSampler createTextureSampler();
        VkPhysicalDeviceProperties properties;

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);


    private:

        std::unique_ptr<Window>& m_Window;

#ifdef NDEBUG
        const bool enableValidationLayers = false;
#else
        const bool enableValidationLayers = false;
#endif
        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();

        bool isDeviceSuitable(VkPhysicalDevice device);
        //Ѱ�������豸�Ķ���
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        //���������豸�Ƿ���������Ҫ����չ
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        std::vector<const char*>  FilterSupportedExtensions(VkPhysicalDevice device,std::vector<std::string>* unsupportedExtensions = nullptr);

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
        std::vector<const char*> getRequiredExtensions();

       

        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkCommandPool commandPool;

        VkDevice device_ = VK_NULL_HANDLE;
        VkSurfaceKHR surface_;
        VkQueue graphicsQueue_;
        VkQueue presentQueue_;

        const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char*> deviceExtensions = { 
                                                            VK_KHR_SWAPCHAIN_EXTENSION_NAME,// ȷ�������� swapchain
                                                            VK_NV_MESH_SHADER_EXTENSION_NAME  // ���� Mesh Shader ��չ 
                                                          };
  
    public:
        inline static std::shared_ptr<VulkanDevice>& GetVulkanDevice() { return m_device; }
        static void Create(std::unique_ptr<Window>& input_window);
    private:
        static std::shared_ptr<VulkanDevice> m_device;
        
    };

}