#include <CommonUnitTest/Common/common.h>

#include "Core/Utils/PathUtils.h"

TEST(PathUtils, GetFilename)
{
	auto name = PrCore::PathUtils::GetFilename("/foo/bar.txt");
	EXPECT_TRUE(name == "bar");
	name = PrCore::PathUtils::GetFilename("/foo/.bar");
	EXPECT_TRUE(name == ".bar");
	name = PrCore::PathUtils::GetFilename("foo.bar.baz.tar");
	EXPECT_TRUE(name == "foo.bar.baz");
	name = PrCore::PathUtils::GetFilename("/foo/bar.");
	EXPECT_TRUE(name == "bar");
	name = PrCore::PathUtils::GetFilename("/foo/bar");
	EXPECT_TRUE(name == "bar");
	name = PrCore::PathUtils::GetFilename("/foo/bar.ext/bar.cc");
	EXPECT_TRUE(name == "bar");
	name = PrCore::PathUtils::GetFilename("/foo/bar.txt/bar.");
	EXPECT_TRUE(name == "bar");
	name = PrCore::PathUtils::GetFilename("/foo/bar.txt/bar");
	EXPECT_TRUE(name == "bar");
	name = PrCore::PathUtils::GetFilename("/foo/.");
	EXPECT_TRUE(name == ".");
	name = PrCore::PathUtils::GetFilename("/foo/..");
	EXPECT_TRUE(name == "..");
	name = PrCore::PathUtils::GetFilename("/foo/.hidden");
	EXPECT_TRUE(name == ".hidden");
	name = PrCore::PathUtils::GetFilename("/foo/..bar");
	EXPECT_TRUE(name == ".");
	name = PrCore::PathUtils::GetFilename("/");
	EXPECT_TRUE(name == "");
	name = PrCore::PathUtils::GetFilename("");
	EXPECT_TRUE(name == "");

	// In Place
	std::string file1{ "foo/bar.ext" };
	auto name1 = PrCore::PathUtils::GetFilenameInPlace(file1);
	EXPECT_TRUE(name1 == "bar");

	file1 = "foo/foo.ext";
	EXPECT_TRUE(name1 == "foo");
}

TEST(PathUtils, ReplaceFilename)
{
	std::string path = "/foo/bar.txt";
	auto replaced = PrCore::PathUtils::ReplaceFilename(path, "foo");
	EXPECT_TRUE(replaced == "/foo/foo.txt");

	path = "/foo/bar.ext/bar.cc";
	replaced = PrCore::PathUtils::ReplaceFilename(path, "foo");
	EXPECT_TRUE(replaced == "/foo/bar.ext/foo.cc");

	path = "/foo/..";
	replaced = PrCore::PathUtils::ReplaceFilename(path, "foo");
	EXPECT_TRUE(replaced == "/foo/foo");

	path = "/";
	replaced = PrCore::PathUtils::ReplaceFilename(path, "foo");
	EXPECT_TRUE(replaced == "/foo");

	path = "/foo.txt";
	replaced = PrCore::PathUtils::ReplaceFilename(path, "");
	EXPECT_TRUE(replaced == "/.txt");

	// Remove
	path = "/foo/bar.ext/bar.cc";
	replaced = PrCore::PathUtils::RemoveFilename(path);
	EXPECT_TRUE(replaced == "/foo/bar.ext/.cc");
}

TEST(PathUtils, GetFile)
{
	auto file = PrCore::PathUtils::GetFile("/foo/bar.txt");
	EXPECT_TRUE(file == "bar.txt");

	file = PrCore::PathUtils::GetFile("/foo/.bar");
	EXPECT_TRUE(file == ".bar");

	file = PrCore::PathUtils::GetFile("/foo/bar/");
	EXPECT_TRUE(file == "");

	file = PrCore::PathUtils::GetFile("/foo/.");
	EXPECT_TRUE(file == ".");

	file = PrCore::PathUtils::GetFile("/foo/..");
	EXPECT_TRUE(file == "..");

	file = PrCore::PathUtils::GetFile(".");
	EXPECT_TRUE(file == ".");

	file = PrCore::PathUtils::GetFile("..");
	EXPECT_TRUE(file == "..");

	file = PrCore::PathUtils::GetFile("/");
	EXPECT_TRUE(file == "");

	file = PrCore::PathUtils::GetFile("//host");
	EXPECT_TRUE(file == "host");

	//In Place
	std::string path = "/foo/bar.txt";
	std::string_view fileInPlace = PrCore::PathUtils::GetFileInPlace(path);
	EXPECT_TRUE(fileInPlace == "bar.txt");

	path = "/foo/foo.txt";
	EXPECT_TRUE(fileInPlace == "foo.txt");
}

TEST(PathUtils, ReplaceFile)
{
	std::string path = "/foo";
	auto replaced = PrCore::PathUtils::PrReplaceFile(path, "bar");
	EXPECT_TRUE(replaced == "/bar");

	path = "/";
	replaced = PrCore::PathUtils::PrReplaceFile(path, "bar");
	EXPECT_TRUE(replaced == "/bar");

	path = "";
	replaced = PrCore::PathUtils::PrReplaceFile(path, "pub");
	EXPECT_TRUE(replaced == "pub");

	path = "/foo";
	replaced = PrCore::PathUtils::PrReplaceFile(path, "");
	EXPECT_TRUE(replaced == "/");

	// Remove
	path = "/foo";
	replaced = PrCore::PathUtils::RemoveFile(path);
	EXPECT_TRUE(replaced == "/");

	// In Place
	path = "/foo/bar.ext/bar.cc";
	std::string_view replacedInplace = PrCore::PathUtils::RemoveFileInPlace(path);
	EXPECT_TRUE(replacedInplace == "/foo/bar.ext/");

	path = "/foo/foo.foo/foo.cc";
	EXPECT_TRUE(replacedInplace == "/foo/foo.foo/");
}
TEST(PathUtils, GetExtension)
{
	auto extension = PrCore::PathUtils::GetExtension("foo/bar.ext");
	EXPECT_TRUE(extension == ".ext");
	extension = PrCore::PathUtils::GetExtension("/foo/bar.");
	EXPECT_TRUE(extension == ".");
	extension = PrCore::PathUtils::GetExtension("/foo/bar");
	EXPECT_TRUE(extension == "");
	extension = PrCore::PathUtils::GetExtension("/foo/bar.ext/bar.cc");
	EXPECT_TRUE(extension == ".cc");
	extension = PrCore::PathUtils::GetExtension("/foo/bar.txt/bar.");
	EXPECT_TRUE(extension == ".");
	extension = PrCore::PathUtils::GetExtension("/foo/bar.txt/bar");
	EXPECT_TRUE(extension == "");
	extension = PrCore::PathUtils::GetExtension("/foo/.");
	EXPECT_TRUE(extension == "");
	extension = PrCore::PathUtils::GetExtension("/foo/..");
	EXPECT_TRUE(extension == "");
	extension = PrCore::PathUtils::GetExtension("/foo/.hidden");
	EXPECT_TRUE(extension == "");
	extension = PrCore::PathUtils::GetExtension("/foo/..bar");
	EXPECT_TRUE(extension == ".bar");

	// In Place
	std::string file{ "foo/bar.ext" };
	auto extension1 = PrCore::PathUtils::GetFilenameInPlace(file);
	EXPECT_TRUE(extension1 == "bar");

	file = "foo/foo.ext";
	EXPECT_TRUE(extension1 == "foo");
}

TEST(PathUtils, ReplaceExtension)
{
	std::string path = "/foo/bar.jpg";
	auto replaced = PrCore::PathUtils::ReplaceExtension(path, ".png");
	EXPECT_TRUE(replaced == "/foo/bar.png");

	path = "/foo/bar.jpg";
	replaced = PrCore::PathUtils::ReplaceExtension(path, "png");
	EXPECT_TRUE(replaced == "/foo/bar.png");

	path = "/foo/bar.jpg";
	replaced = PrCore::PathUtils::ReplaceExtension(path, ".");
	EXPECT_TRUE(replaced == "/foo/bar.");

	path = "/foo/bar.jpg";
	replaced = PrCore::PathUtils::ReplaceExtension(path, "");
	EXPECT_TRUE(replaced == "/foo/bar");

	path = "/foo/bar.";
	replaced = PrCore::PathUtils::ReplaceExtension(path, "png");
	EXPECT_TRUE(replaced == "/foo/bar.png");

	path = "/foo/bar";
	replaced = PrCore::PathUtils::ReplaceExtension(path, ".png");
	EXPECT_TRUE(replaced == "/foo/bar.png");

	path = "/foo/bar";
	replaced = PrCore::PathUtils::ReplaceExtension(path, "png");
	EXPECT_TRUE(replaced == "/foo/bar.png");

	path = "/foo/bar";
	replaced = PrCore::PathUtils::ReplaceExtension(path, ".");
	EXPECT_TRUE(replaced == "/foo/bar.");

	path = "/foo/bar";
	replaced = PrCore::PathUtils::ReplaceExtension(path, "");
	EXPECT_TRUE(replaced == "/foo/bar");

	path = "/foo/.";
	replaced = PrCore::PathUtils::ReplaceExtension(path, ".png");
	EXPECT_TRUE(replaced == "/foo/..png");

	path = "/foo/.";
	replaced = PrCore::PathUtils::ReplaceExtension(path, "png");
	EXPECT_TRUE(replaced == "/foo/..png");

	path = "/foo/.";
	replaced = PrCore::PathUtils::ReplaceExtension(path, ".");
	EXPECT_TRUE(replaced == "/foo/..");

	path = "/foo/.";
	replaced = PrCore::PathUtils::ReplaceExtension(path, "");
	EXPECT_TRUE(replaced == "/foo/.");

	path = "/foo/.";
	replaced = PrCore::PathUtils::ReplaceExtension(path, "");
	EXPECT_TRUE(replaced == "/foo/.");

	path = "/foo/";
	replaced = PrCore::PathUtils::ReplaceExtension(path, ".png");
	EXPECT_TRUE(replaced == "/foo/.png");

	path = "/foo/";
	replaced = PrCore::PathUtils::ReplaceExtension(path, "png");
	EXPECT_TRUE(replaced == "/foo/.png");

	// Remove
	path = "/foo/bar.ext/bar.cc";
	replaced = PrCore::PathUtils::RemoveExtension(path);
	EXPECT_TRUE(replaced == "/foo/bar.ext/bar");

	path = "/foo/bar.ext";
	replaced = PrCore::PathUtils::RemoveExtension(path);
	EXPECT_TRUE(replaced == "/foo/bar");

	// In Place
	path = "/foo/bar.ext/bar.cc";
	std::string_view replacedInplace = PrCore::PathUtils::RemoveExtensionInPlace(path);
	EXPECT_TRUE(replacedInplace == "/foo/bar.ext/bar");

	path = "/foo/foo.foo/foo.cc";
	EXPECT_TRUE(replacedInplace == "/foo/foo.foo/foo");
}

TEST(PathUtils, MakePath)
{
	std::string path = PrCore::PathUtils::MakePath("foo", "bar.png");
	EXPECT_TRUE(path == "foo/bar.png");

	path = PrCore::PathUtils::MakePath("", "bar.png");
	EXPECT_TRUE(path == "/bar.png");

	path = PrCore::PathUtils::MakePath("foo/", "bar.png");
	EXPECT_TRUE(path == "foo/bar.png");

	path = PrCore::PathUtils::MakePath("foo/", "/bar.png");
	EXPECT_TRUE(path == "foo/bar.png");

	path = PrCore::PathUtils::MakePath("", "");
	EXPECT_TRUE(path == "/");

	path = PrCore::PathUtils::MakePath("foo", "bar", ".png");
	EXPECT_TRUE(path == "foo/bar.png");

	path = PrCore::PathUtils::MakePath("foo/", "/bar.", ".png");
	EXPECT_TRUE(path == "foo/bar.png");

	path = PrCore::PathUtils::MakePath("foo", "bar", "png");
	EXPECT_TRUE(path == "foo/bar.png");

	path = PrCore::PathUtils::MakePath("foo", "", "");
	EXPECT_TRUE(path == "foo/.");

	path = PrCore::PathUtils::MakePath("", "", "");
	EXPECT_TRUE(path == "/.");
}

TEST(PathUtils, SplitPath)
{
	auto pathVec = PrCore::PathUtils::SplitPath("foo/bar.png");
	EXPECT_TRUE(pathVec[0] == "foo");
	EXPECT_TRUE(pathVec[1] == "bar.png");

	pathVec = PrCore::PathUtils::SplitPath("foo/foo/bar.png");
	EXPECT_TRUE(pathVec[0] == "foo");
	EXPECT_TRUE(pathVec[1] == "foo");
	EXPECT_TRUE(pathVec[2] == "bar.png");

	pathVec = PrCore::PathUtils::SplitPath("foo/foo/");
	EXPECT_TRUE(pathVec[0] == "foo");
	EXPECT_TRUE(pathVec[1] == "foo");
	EXPECT_TRUE(pathVec[2] == "");

	pathVec = PrCore::PathUtils::SplitPath("foo/bar");
	EXPECT_TRUE(pathVec[0] == "foo");
	EXPECT_TRUE(pathVec[1] == "bar");

	// In Place
	std::string path = "foo/bar.png";
	auto pathInPlaceVec = PrCore::PathUtils::SplitPathInPlace(path);
	EXPECT_TRUE(pathInPlaceVec[0] == "foo");
	EXPECT_TRUE(pathInPlaceVec[1] == "bar.png");

	path = "bar/bar.txt";
	EXPECT_TRUE(pathInPlaceVec[0] == "bar");
	EXPECT_TRUE(pathInPlaceVec[1] == "bar.txt");


	path = "foo/foo/bar.png";
	pathInPlaceVec = PrCore::PathUtils::SplitPathInPlace(path);
	EXPECT_TRUE(pathInPlaceVec[0] == "foo");
	EXPECT_TRUE(pathInPlaceVec[1] == "foo");
	EXPECT_TRUE(pathInPlaceVec[2] == "bar.png");

	path = "bar/bar/foo.txt";
	EXPECT_TRUE(pathInPlaceVec[0] == "bar");
	EXPECT_TRUE(pathInPlaceVec[1] == "bar");
	EXPECT_TRUE(pathInPlaceVec[2] == "foo.txt");


	path = "foo/foo/";
	pathInPlaceVec = PrCore::PathUtils::SplitPathInPlace(path);
	EXPECT_TRUE(pathInPlaceVec[0] == "foo");
	EXPECT_TRUE(pathInPlaceVec[1] == "foo");
	EXPECT_TRUE(pathInPlaceVec[2] == "");

	path = "bar/bar/foo.txt";
	EXPECT_TRUE(pathInPlaceVec[0] == "bar");
	EXPECT_TRUE(pathInPlaceVec[1] == "bar");
	EXPECT_TRUE(pathInPlaceVec[2] == "");
}

TEST(PathUtils, GetSubFolder)
{
	std::string subFolder = PrCore::PathUtils::GetSubFolder("foo/bar/baz.txt");
	EXPECT_TRUE(subFolder == "bar");
	subFolder = PrCore::PathUtils::GetSubFolder("foo/bar/baz.txt", 1);
	EXPECT_TRUE(subFolder == "foo");

	subFolder = PrCore::PathUtils::GetSubFolder("foo/bar");
	EXPECT_TRUE(subFolder == "foo");
}