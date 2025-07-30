#include"VulkanDevice.h"
#include"GLFW/glfw3.h"
namespace Graffiti {
	
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}
	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}
	VulkanDevice::VulkanDevice(std::unique_ptr<Window>& input_window):m_Window(input_window)
	{
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createCommandPool();
	}
	VulkanDevice::~VulkanDevice()
	{
		
		if (!enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}
		vkDestroyInstance(instance,nullptr);
	}
	uint32_t VulkanDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties); 
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!"); 
	}
	VkFormat VulkanDevice::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (
				tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}
		throw std::runtime_error("failed to find supported format!");
	}
	void* mx = nullptr;
	void VulkanDevice::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
	
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	
        if (vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create vertex buffer!");
        }
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size; 
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
	
        if (vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate vertex buffer memory!");
        }
		vkBindBufferMemory(device_, buffer, bufferMemory, 0);
		
		
	}
	void VulkanDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;  // Optional
		copyRegion.dstOffset = 0;  // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}
	void VulkanDevice::createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		if (vkCreateImage(device_, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device_, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		if (vkBindImageMemory(device_, image, imageMemory, 0) != VK_SUCCESS) {
			throw std::runtime_error("failed to bind image memory!");
		}
	}
	void VulkanDevice::copyImage(VkBuffer srcBuffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		transitionImageLayout(commandBuffer, image,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0; // tightly packed
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(commandBuffer, srcBuffer, image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &region);

		// 3. ת��ͼ��Ϊ SHADER_READ_ONLY_OPTIMAL
		transitionImageLayout(commandBuffer, image,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		endSingleTimeCommands(commandBuffer);
	}

	void VulkanDevice::transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image,
		VkFormat format,
		VkImageLayout oldLayout, VkImageLayout newLayout) {
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		// �������Ϸ���Ȩ�ޣ����ݲ��ֱ仯�趨��
		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}

	VkImageView VulkanDevice::createImageView( VkImage image, VkFormat format)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;

		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // 2D ����
		viewInfo.format = format;

		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;      // ���ʹ���� mipmap��������Ҫ�� mip ����
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(device_, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	VkSampler VulkanDevice::createTextureSampler()
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // ��̬��ȡ���ֵ����֤����

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f; // ������ mipmap������Ҫ��� mip levels ����

		VkSampler textureSampler;
		if (vkCreateSampler(device_, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}

		return textureSampler;
	}

	VkCommandBuffer VulkanDevice::beginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		return commandBuffer;
	}
	void VulkanDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue_);
		vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);
	}
	void VulkanDevice::createInstance()
	{

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Graffiti";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Graffiti Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_1;



		VkInstanceCreateInfo createInfo = { };
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();



		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			GF_CORE_ERROR("Failed to Create Instance, File:VulaknDevice.cpp");
		}
    
	}
	void VulkanDevice::setupDebugMessenger()
	{
		if (!enableValidationLayers) return;
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;  // Optional

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			GF_CORE_ERROR("Failed to set up debug messenger! File:VulaknDevice.cpp");
		}

	}
	void VulkanDevice::createSurface()
	{
		
		if (glfwCreateWindowSurface(instance, static_cast<GLFWwindow*>( m_Window->GetNativeWindow()), nullptr, &surface_) != VK_SUCCESS) {
			GF_CORE_ERROR("Failed to craete window surface! File:VulaknDevice.cpp");
		}	
	}
	//ѡ���ʺϵ������豸 
	bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice device) {

		//��ѯ�����豸�Ķ���
		QueueFamilyIndices indices = findQueueFamilies(device);
		
		//�ж������豸�Ƿ�֧����Ҫ����չ
		bool extensionsSupported = checkDeviceExtensionSupport(device);
	
		//��ѯ�����豸�Ƿ�֧�ֽ�������ĳЩ����
		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}
		//��ȡ�����豸������
		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
	
		//������Ҫ�Ĺ��ܶ�֧�ֵ������豸������������Ҫ�������豸
		return indices.isComplete() && extensionsSupported && swapChainAdequate &&
			supportedFeatures.samplerAnisotropy;
	}
	void VulkanDevice::pickPhysicalDevice()
	{
		//���ε���vkEnumeratePhysicalDevices������ȡ�豸��Ϣ��
	  //���Ӧ����c���Է��Ĵ��룬
	  //��һ�λ�ȡ�������ڶ��θ������������ȡ��Ӧ��������Ϣ
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			GF_CORE_ERROR("Faild to find GPU with Vulkan support!  File:VulaknDevice.cpp");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}
		if (physicalDevice == VK_NULL_HANDLE) {
			physicalDevice = devices[0];
			GF_CORE_WARN("no suitalbe PhysicalDevice!");
		}
		vkGetPhysicalDeviceProperties(physicalDevice, &properties); 
	}
	void VulkanDevice::createLogicalDevice()
	{
		//��ȡ�����豸�Ķ�������
		
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };
	
		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		
		 VkPhysicalDeviceMeshShaderFeaturesNV meshShaderFeatures{};
		 meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;

		 VkPhysicalDeviceFeatures2 features2{};
		 features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		 features2.pNext = &meshShaderFeatures; // �� Mesh Shader ���Խṹ���ӵ����Բ�ѯ�� 
		 //
		 vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);

		std::vector<std::string> unsupported;
		std::vector<const char*> enabledExtensions = FilterSupportedExtensions(physicalDevice, &unsupported);
		// ��ӡһ����Щ��չ������
		for (const auto& ext : unsupported) {
			GF_CORE_WARN( "Device does not support extension: {0}" , ext);
		}
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = &features2;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = NULL;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
		createInfo.ppEnabledExtensionNames = enabledExtensions.data();

		// might not really be necessary anymore because device specific validation layers
		// have been deprecated
	/*	if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}*/

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
		vkGetDeviceQueue(device_, indices.presentFamily,  0, &presentQueue_);
	}
	void VulkanDevice::createCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
		poolInfo.flags =
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}
	QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice device)
	{
		
			QueueFamilyIndices indices;

			//���ǺͲ�ѯ�����豸����������Ϣһ���ķ�ʽȥ��ѯ�����豸�Ķ�����������Ϣ
			uint32_t queueFamilyCount = 0;
	
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
			
			int i = 0;
			//���������豸��ÿһ������
			for (const auto& queueFamily : queueFamilies) {
				//�ж���������Ƿ����0����������Ϊ�����������пն��У�
				//�ж���������Ƿ�֧��ͼ�ζ���
				
				if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					indices.graphicsFamily = i;
					indices.graphicsFamilyHasValue = true;
				}
				VkBool32 presentSupport = false;
				//�ж���������Ƿ�֧�ֳ��ֶ���
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
				if (queueFamily.queueCount > 0 && presentSupport) {
					indices.presentFamily = i;
					indices.presentFamilyHasValue = true;
				}
				//����Ѿ�����������Ҫ��ͼ�ζ��к���ʾ���У���ô���Ǿ��˳�ѭ��������ѡ���˶���
				if (indices.isComplete()) {
					break;
				}
				//Ҫ��û��ѡ�ã��Ǿ����±���
				i++;
			}
			//���ض�����Ϣ
			return indices;
		
	}
	bool VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		
			uint32_t extensionCount;
			//�����������ε��õķ�ʽȥ��ȡ�����豸��չ����Ϣ
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(
				device,
				nullptr,
				&extensionCount,
				availableExtensions.data());

			std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

			//������˼����ѡ��ʽ��
			//��������豸����������Ҫ����չ�ͰѶ�Ӧ��չ�����ֲ���
			//������������û��requiredExtensions�գ�Ҫ�ǿ���˵�����������ˣ�����
			for (const auto& extension : availableExtensions) {
				requiredExtensions.erase(extension.extensionName);
			}
			return requiredExtensions.empty();
		
	}
	std::vector<const char*> VulkanDevice::FilterSupportedExtensions(VkPhysicalDevice device, std::vector<std::string>* unsupportedExtensions )
	{
		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> availableSet;
		for (const auto& ext : availableExtensions) {
			availableSet.insert(ext.extensionName);
		}

		std::vector<const char*> enabledExtensions;
		for (const char* req : deviceExtensions) {
			if (availableSet.count(req)) {
				enabledExtensions.push_back(req); // ֧�ֵļ���
			}
			else if (unsupportedExtensions) {
				unsupportedExtensions->emplace_back(req); // ��֧�ֵ�Ҳ��¼һ�£���ѡ��
			}
		}

		return enabledExtensions;
	}
	SwapChainSupportDetails VulkanDevice::querySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				device,
				surface_,
				&presentModeCount,
				details.presentModes.data());
		}
		return details;
	}
	std::vector<const char*> VulkanDevice::getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			 extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		} 

		return extensions;
	}

	std::shared_ptr<VulkanDevice> VulkanDevice::m_device = nullptr;

	void VulkanDevice::Create(std::unique_ptr<Window>& input_window)
	{
		if (m_device == nullptr) {
			m_device = std::make_shared<VulkanDevice>(input_window);
		}
	}
}


