#include "lemken_zirkon12_ddop.hpp"

#include "isobus/isobus/isobus_device_descriptor_object_pool.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

int main(int argc, char **argv)
{
	const std::string outputPath = (argc > 1) ? argv[1] : "Zirkon12DDOP.iop";
	// Generate a DDOP compatible with ISO 11783-10 Task Controller version 3.
	auto pool = std::make_shared<isobus::DeviceDescriptorObjectPool>(3);

	if (!LemkenZirkon12DDOP::create_ddop(pool, isobus::NAME(0)))
	{
		std::cerr << "Failed to create the DDOP" << std::endl;
		return 1;
	}

	std::vector<std::uint8_t> binaryPool;
	if (!pool->generate_binary_object_pool(binaryPool))
	{
		std::cerr << "Failed to serialize the DDOP" << std::endl;
		return 2;
	}

	std::ofstream file(outputPath, std::ios::binary);
	if (!file)
	{
		std::cerr << "Failed to open output file: " << outputPath << std::endl;
		return 3;
	}

	file.write(reinterpret_cast<const char *>(binaryPool.data()),
	           static_cast<std::streamsize>(binaryPool.size()));
	if (!file)
	{
		std::cerr << "Failed to write output file: " << outputPath << std::endl;
		return 4;
	}

	std::cout << "Wrote " << outputPath << ", " << binaryPool.size() << " bytes" << std::endl;
	return 0;
}