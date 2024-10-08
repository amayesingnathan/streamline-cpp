#pragma once

#include <slc/Types/Buffer.h>

namespace fs = std::filesystem;

namespace slc {

	class FileUtils
	{
	public:
		static fs::path LabRoot();

		static Buffer Read(const fs::path& filepath);
		static void Read(const fs::path& filepath, std::string& string);

		static void Write(const fs::path& filepath, Buffer buffer);
		static void Write(const fs::path& filepath, std::string_view string);

		static void Create(const fs::path& filepath);
		static void CreateDir(const fs::path& filepath);

		static void CopyDir(const fs::path& src, const fs::path& dest);

		static void Remove(const fs::path& filepath);
		static void RemoveDir(const fs::path& filepath);

		// These return empty strings if cancelled
		static fs::path OpenFile(const std::vector<std::string>& filter);
		static fs::path OpenDir();
		static fs::path SaveFile(const std::vector<std::string>& filter);
	};

}