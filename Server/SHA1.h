#pragma once

#include <string>
#include <vector>
#include <cstdint>

class SHA1
{
private:
	mutable uint32_t h0, h1, h2, h3, h4;

	void processBlock(const uint8_t* block) const;
	void padMessage(std::vector <uint8_t>& message) const;
	std::string digestToString() const;

public:
	SHA1();
	std::string hash(const std::string& input) const;
};

