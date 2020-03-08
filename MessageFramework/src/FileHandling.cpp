#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <chrono>
#include <thread>

#include "ClasslessLogger.hpp"
#include "LogSpace.hpp"
#include "FileHandling.hpp"

namespace
{
enum class FileMode
{
    toWrite,
    toRead
};

bool createGuardian(const std::string& pathToFolder)
{
    fileLog(("FileInterface::createGuardian " + pathToFolder).c_str(), LogSpace::FileHandling);
    FileInterface::Managment::createFile(pathToFolder + "/GUARD");
    return FileInterface::Managment::isFileExist(pathToFolder + "/GUARD");
}

bool removeGuardian(const std::string& pathToFolder)
{
    fileLog(("FileInterface::createGuardian " + pathToFolder).c_str(), LogSpace::FileHandling);
    remove((pathToFolder + "/GUARD").c_str());
    return ! FileInterface::Managment::isFileExist(pathToFolder + "/GUARD");
}

bool isGuardianExist(const std::string& pathToFolder)
{
    return FileInterface::Managment::isFileExist(pathToFolder + "/GUARD");
}

void waitForAccess(const std::string& folderName)
{
    bool accesToFile = false;
    fileLog(("FileInterface::waitForAccess START in FileMode = " + folderName).c_str(), LogSpace::FileHandling);
    while(!accesToFile)
    {
        if (!isGuardianExist(folderName))
        {
            createGuardian(folderName);
            accesToFile = true;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    fileLog(("FileInterface::waitForAccess END in FileMode = " + folderName).c_str(), LogSpace::FileHandling);
}

std::unique_ptr<std::fstream> openFile(const std::string& pathToFile, FileMode mode, AccesMode accesMode = AccesMode::withGuardian)
{
    fileLog(("FileInterface::openFile started in FileMode = " + std::to_string(static_cast<int>(mode))).c_str(), LogSpace::FileHandling);
    if (!FileInterface::Managment::isFileExist(pathToFile))
    {
        std::string logInfo = "FileInterface::openFile ERROR: " + pathToFile + " does not exist";
        fileLog(logInfo.c_str(), LogSpace::FileHandling);
        return nullptr;
    }

    std::string folderName = *FileInterface::Accesor::getFolderName(pathToFile);

    if(AccesMode::withGuardian == accesMode)
    {
        waitForAccess(folderName);
    }

    std::unique_ptr<std::fstream> fileToOpen= std::make_unique<std::fstream>();

    if (FileMode::toRead == mode)
    {
        fileToOpen->open(pathToFile, std::ios::in);
    }
    else if (FileMode::toWrite == mode)
    {
        fileToOpen->open(pathToFile, std::ios::out | std::ios::app);
    }

    if (fileToOpen->is_open())
    {
        return fileToOpen;
    }
    else
    {
        fileLog("FileInterface::openFile ERROR: is_open() failed", LogSpace::FileHandling);
        return nullptr;
    }
}

std::unique_ptr<std::fstream> openFileToWrite(const std::string& pathToFile, AccesMode accesMode = AccesMode::withGuardian)
{
    return openFile(pathToFile, FileMode::toWrite, accesMode);
}

std::unique_ptr<std::fstream> openFileToRead(const std::string& pathToFile, AccesMode accesMode = AccesMode::withGuardian)
{
    return openFile(pathToFile, FileMode::toRead, accesMode);
}
}//namespace

bool FileInterface::Modification::addRow(const std::string& pathToFile, const std::string& text)
{
    fileLog(("FileInterface::Modification::addRow Add row to " + pathToFile).c_str(), LogSpace::FileHandling);
    std::string folderName = *Accesor::getFolderName(pathToFile);
    if (std::unique_ptr<std::fstream> file = openFileToWrite(pathToFile))
    {
        *file << text;
        *file << '\n';
        removeGuardian(folderName);
        return true;
    }
    else
    {
        fileLog("FileInterface::Modification::addRow ERROR: Cannot get file acces", LogSpace::FileHandling);
        removeGuardian(folderName);
        return false;
    }
}


bool FileInterface::Managment::createFile(const std::string& pathToFile)
{
    if (isFileExist(pathToFile))
    {
        std::string logInfo = "FileInterface::Managment::createFile() started ERROR: " + pathToFile + "File exist!";
        fileLog(logInfo.c_str(), LogSpace::FileHandling);
        return false;
    }
    std::string logInfo = "FileInterface::Managment::createFile() -> " + pathToFile;
    fileLog(logInfo.c_str(), LogSpace::FileHandling);

    std::string systemCommand = "touch " + pathToFile;
    system("umask 777");
    system(systemCommand.c_str());

    if (!isFileExist(pathToFile))
    {
        std::string logInfo = "FileInterface::Managment::createFile started ERROR: " + pathToFile + "was not create";
        fileLog(logInfo.c_str(), LogSpace::FileHandling);
        return false;
    }

    return true;
}

std::unique_ptr<std::vector<std::string>> FileInterface::Accesor::getFileContent(const std::string& pathToFile, AccesMode accesMode = AccesMode::withGuardian)
{
    std::string logInfo = "FileInterface::Accesor::getFileContent() " + pathToFile;
    fileLog(logInfo.c_str(), LogSpace::FileHandling);
    std::unique_ptr<std::vector<std::string>> fileContent = std::make_unique<std::vector<std::string>>();
    std::string folderName = *Accesor::getFolderName(pathToFile);

    if (std::unique_ptr<std::fstream> file = openFileToRead(pathToFile, accesMode))
    {
        while (!file->eof())
        {
            std::string row;
            std::getline(*file, row);
            fileContent -> push_back(row);
        }
    }
    else
    {
        std::string logInfo = "FileInterface::Accesor::getFileContent ERROR: Cannot get acces to "  + pathToFile;
        fileLog(logInfo.c_str(), LogSpace::FileHandling);
        if (AccesMode::withGuardian == accesMode)
        {
            removeGuardian(folderName);
        }
        return nullptr;
    }

    if (AccesMode::withGuardian == accesMode)
    {
        removeGuardian(folderName);
    }

    return fileContent;
}

std::unique_ptr<std::vector<std::string>> FileInterface::Accesor::getFilenamesFromFolder(const std::string& pathToDir)
{
    std::string command = "ls " + pathToDir;
    std::string commandOutput = ConsolControl::getStdoutFromStartedCommand(command);

    std::unique_ptr<std::vector<std::string>> filesNames = std::make_unique<std::vector< std::string>>();
    std::string fileName;

    auto it = commandOutput.begin();
    for (auto& x : commandOutput)
    {
        if (commandOutput.end() - 1 == it)
        {
            fileName += x;
            filesNames->push_back(fileName);
        }
        else if ('\n' != x)
        {
            fileName += x;
        }
        else
        {
            filesNames->push_back(fileName);
            fileName.clear();
        }
        it++;
    }

    return filesNames;
}

std::unique_ptr<std::string> FileInterface::Accesor::getFolderName(const std::string& pathToFile)
{
    auto it = pathToFile.end();

    while ('/' != *it)
    {
        --it;
    }
    std::unique_ptr<std::string> folderName = std::make_unique<std::string>(pathToFile.begin(), it);
    return folderName;
}


std::unique_ptr<std::string> FileInterface::Accesor::getRowField(const std::string& field, const int fieldNumber)
{
    fileLog("FileInterface::Accesor::getRowField() started from ", LogSpace::FileHandling);
    int actualFieldNumber = -1;
    std::unique_ptr<std::string> fieldToDownload = std::make_unique<std::string>();

    for (auto& x : field)
    {
        if ('[' == x)
        {
            ++actualFieldNumber;
        }
        else if (actualFieldNumber == fieldNumber && ']' != x)
        {
            *fieldToDownload += x;
        }
    }
    return fieldToDownload;
}

std::unique_ptr<std::string> FileInterface::Accesor::getRow(const std::string& pathToFile, const std::string& pattern)
{
    std::string folderName = *Accesor::getFolderName(pathToFile);
    waitForAccess(folderName);

    std::string command = "grep '" + pattern + "' " +  pathToFile;
    std::string commandOutput = ConsolControl::getStdoutFromStartedCommand(command);

    removeGuardian(folderName);

    if (commandOutput.empty())
    {
        return nullptr;
    }

    return std::make_unique<std::string>(commandOutput);
}

bool FileInterface::Managment::isFileExist(const std::string& pathToFile)
{
    bool isFileExist = (-1 != access(pathToFile.c_str(), F_OK ));
    return isFileExist;
}

bool FileInterface::Managment::removeFile(const std::string& pathToFile)
{
    std::string logInfo = "FileInterface::Managment::removeFile() -> " + pathToFile;
    fileLog(logInfo.c_str(), LogSpace::FileHandling);
    remove(pathToFile.c_str());
    return ! isFileExist(pathToFile);
}

bool FileInterface::Modification::removeRow(const std::string& pathToFile, const std::string& pattern)
{
    std::string logInfo = "FileInterface::Modification::removeRow() -> " + pathToFile;
    fileLog(logInfo.c_str(), LogSpace::FileHandling);

    if (nullptr == Accesor::getRow(pathToFile, pattern))
    {
        const std::string logInfo = "FileInterface::Modification::removeRow() -> Attempt to remove unexist row!";
        fileLog(logInfo.c_str(), LogSpace::FileHandling);
        return false;
    }

    std::string folderName = *Accesor::getFolderName(pathToFile);
    waitForAccess(folderName);

    std::string command = "sed -i -e '/" + pattern + "/d' " + pathToFile;
    std::system(command.c_str());

    removeGuardian(folderName);

    const auto isRowRemoved = Accesor::getRow(pathToFile, pattern);
    return nullptr == isRowRemoved;
}

bool FileInterface::Modification::updateRow(const std::string & pathToFile, const std::string & newRow, const std::string & where)
{
    std::string folderName = *Accesor::getFolderName(pathToFile);
    waitForAccess(folderName);

    std::string command = "sed -i -e '0,/" + where + "/s/.*" + where + ".*/" + newRow + "/g' " + pathToFile;
    std::system(command.c_str());

    bool isGuardRemoved = removeGuardian(folderName);
    return isGuardRemoved;
}

bool FileInterface::Modification::updateFlagsInFile(const std::string& pathToFile, const std::string& flagToReplace, const std::string& newFlag)
{
    std::string command = "sed -i -e '/\\[" + flagToReplace + "]/{s/" + flagToReplace + "/" + newFlag + "/}' " + pathToFile;
    std::system(command.c_str());

    return true;
}

bool FileInterface::Modification::updateRowField(const std::string& pathToFile, const std::string& where, const std::string& newField, const int fieldNumber = 0)
{
    int actualFieldNumber = -1;

    std::string folderName = *Accesor::getFolderName(pathToFile);
    waitForAccess(folderName);

    std::string command = "grep '" + where + "' " + pathToFile;
    std::string row = ConsolControl::getStdoutFromStartedCommand(command.c_str());

    std::string rowToUpdate;
    int oldFieldSize = 0;

    for (auto it = row.begin(); it != row.end();)
    {
        if ('[' == *it)
        {
            ++actualFieldNumber;
        }
        else if (actualFieldNumber == fieldNumber)
        {
            oldFieldSize++;
        }

        if (actualFieldNumber == fieldNumber && ']' == *it)
        {
            rowToUpdate = std::string(row.begin(), it - oldFieldSize) + "[" + newField;
            rowToUpdate += std::string(it, row.end());
            break;
        }
        it++;
    }

    removeGuardian(folderName);
    command = "sed -i -e '0,/" + where + "/s/.*" + where + ".*/" + rowToUpdate + "/g' " + pathToFile;
    std::system(command.c_str());
    return true;
}

bool FileInterface::lockFolder(const std::string& pathToFolder)
{
    waitForAccess(pathToFolder);
    return FileInterface::Managment::isFileExist(pathToFolder + "/GUARD");
}

bool FileInterface::unlockFolder(const std::string& pathToFolder)
{
    Managment::removeFile(pathToFolder + "/GUARD");
    return ! FileInterface::Managment::isFileExist(pathToFolder + "/GUARD");
}

/** TO NIZEJ GDIZE INDZIEJ*/

std::string ConsolControl::getStdoutFromStartedCommand(std::string cmd)
{
    std::string data;
    FILE * stream;
    const int max_buffer = 1024;
    char buffer[max_buffer];
    cmd.append(" 2>&1");
    stream = popen(cmd.c_str(), "r");
    if (stream)
    {
        while (!feof(stream))
        {
            if (fgets(buffer, max_buffer, stream) != NULL)
            {
                data.append(buffer);
            }
        }
        pclose(stream);
    }
    if (data.size())
        data.pop_back(); //usuwanie smieci konca linii
    return data;
}
