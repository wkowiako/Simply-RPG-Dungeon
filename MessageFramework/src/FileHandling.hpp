#pragma once
#include<memory>
#include<vector>
#include<string>
#include<fstream>

enum class AccesMode
{
    withGuardian,
    withoutGuardian
};

namespace FileInterface
{
    namespace Modification
    {
        bool addRow(const std::string& pathToFile, const std::string& text);
        bool removeRow(const std::string& pathToFile, const std::string& pattern);
        bool updateRow(const std::string& pathToFile, const std::string& newRow, const std::string& where);
        bool updateFlagsInFile(const std::string& pathToFile, const std::string& flagToReplace, const std::string& newFlag);
        bool updateRowField(const std::string& pathToFile, const std::string& where, const std::string& newField, const int fieldNumber);
    }

    namespace Accesor
    {
        std::unique_ptr<std::vector<std::string>> getFilenamesFromFolder(const std::string& pathToDir);
        std::unique_ptr<std::string> getFolderName(const std::string& pathToFile);
        std::unique_ptr<std::string> getRowField(const std::string& field, const int fieldNumber);
        std::unique_ptr<std::vector<std::string>> getFileContent(const std::string& pathToFile, AccesMode accesMode);
        std::unique_ptr<std::string> getRow(const std::string& pathToFile, const std::string& pattern);
    }

    namespace Managment
    {
        bool createFile(const std::string& pathToFile);
        bool removeFile(const std::string& pathToFile);
        bool isFileExist(const std::string& pathToFile);
    }

    bool lockFolder(const std::string& pathToFolder);
    bool unlockFolder(const std::string& pathToFolder);
}

/** wrzucic to gdzie indzie*/
namespace ConsolControl
{
    std::string getStdoutFromStartedCommand(std::string cmd);
}



