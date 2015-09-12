#pragma once

#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>

// 고정 사이즈 (헤더 + 페이로드) 버퍼를 지닌 message wrapper
class chat_message
{
public:
	enum { header_length = 4 };
	enum { max_body_length = 512 };

	chat_message()
		: body_length_(0)
	{
	}

	const char* data() const
	{
		return data_;
	}

	char* data()
	{
		return data_;
	}

	// 총 크기 = 패킷 크기 + 페이로드 크기
	std::size_t length() const
	{
		return header_length + body_length_;
	}

	// 페이로드 리턴
	const char* body() const
	{
		return data_ + header_length;
	}

	// 페이로드 리턴
	char* body()
	{
		return data_ + header_length;
	}

	// 페이로드 크기
	std::size_t body_length() const
	{
		return body_length_;
	}

	void body_length(std::size_t new_length)
	{
		body_length_ = new_length;

		if (body_length_ > max_body_length)
		{
			body_length_ = max_body_length;
		}

	}

	bool decode_header()
	{
		// 헤더만 따로 뽑기
		char header[header_length + 1] = "";
		std::strncat(header, data_, header_length);

		// 헤더에는 페이로드 사이즈가 담겨 있음
		body_length_ = std::atoi(header);

		// 페이로드 사이즈 검증
		if (body_length_ > max_body_length)
		{
			body_length_ = 0;
			return false;
		}
		return true;
	}

	void encode_header()
	{
		// 4바이트 헤더 붙여주기
		char header[header_length + 1] = "";
		std::sprintf(header, "%4d", static_cast<int>(body_length_));
		std::memcpy(data_, header, header_length);
	}

private:
	// 516바이트 고정크기
	char data_[header_length + max_body_length];
	std::size_t body_length_;
};

#endif // CHAT_MESSAGE_HPP