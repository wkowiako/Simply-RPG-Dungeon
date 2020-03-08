#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

#include "FileHandling.hpp"
#include "GlobalVariables.hpp"

const std::string filename = "test.txt";
const std::string testPath = ENVIRONMENT_PATH::TO_FOLDER::TEST;

TEST(FileHandlingTest, createFile)
{
    int isFileExist = access(filename.c_str(), F_OK );
    ASSERT_FALSE(-1 != isFileExist);

    FileInterface::Managment::createFile(filename);

    isFileExist = access(filename.c_str(), F_OK );
    ASSERT_TRUE(-1 != isFileExist);

    std::string command = "rm " + filename;
    system(command.c_str());
}

TEST(FileHandlingTest, isFileExist)
{
    EXPECT_FALSE(FileInterface::Managment::isFileExist(filename));

    FileInterface::Managment::createFile(filename);

    EXPECT_TRUE(FileInterface::Managment::isFileExist(filename));

    std::string command = "rm " + filename;
    system(command.c_str());
}

TEST(FileHandlingTest, removeFile)
{
    FileInterface::Managment::createFile(filename);

    EXPECT_TRUE(FileInterface::Managment::isFileExist(filename));

    FileInterface::Managment::removeFile(filename);

    ASSERT_FALSE(FileInterface::Managment::isFileExist(filename));
}


TEST(FileHandlingTest, getFileContent)
{
    std::string testFile = testPath + filename;
    FileInterface::Managment::createFile(testFile);
    auto  fileContent = FileInterface::Accesor::getFileContent(testFile, AccesMode::withGuardian);

    EXPECT_EQ(fileContent->at(0), "");

    std::string firstRow = "[marcin][13][323]";
    std::string secondRow = "[nurzyn][4][23aa]";
    std::string command_1 = "echo " + firstRow + " >> " + testFile;
    std::string command_2 = "echo " + secondRow + " >> " + testFile;
    system(command_1.c_str());
    system(command_2.c_str());
    fileContent = FileInterface::Accesor::getFileContent(testFile, AccesMode::withGuardian);

    EXPECT_EQ(fileContent->at(0), firstRow);
    EXPECT_EQ(fileContent->at(1), secondRow);

    FileInterface::Managment::removeFile(testFile);
}

TEST(FileHandlingTest, getRowField)
{
    std::string row = "[marcin][3][ddd]";
    std::string rowField_1;
    std::string rowField_2;
    std::string rowField_3;

    rowField_1 = *FileInterface::Accesor::getRowField(row,0);
    rowField_2 = *FileInterface::Accesor::getRowField(row,1);
    rowField_3 = *FileInterface::Accesor::getRowField(row,2);

    EXPECT_EQ(rowField_1, "marcin");
    EXPECT_EQ(rowField_2, "3");
    EXPECT_EQ(rowField_3, "ddd");
    EXPECT_EQ(row, "[marcin][3][ddd]");
}

TEST(FileHandlingTest, getFolderName)
{
    std::string filenameWithPath = testPath + filename;
    std::string foldername = *FileInterface::Accesor::getFolderName(filenameWithPath);

    EXPECT_EQ(foldername + "/", testPath);
}

TEST(FileHandlingTest, getFilenamesFromFolder)
{
    auto filenamesFormFolder = FileInterface::Accesor::getFilenamesFromFolder(testPath);
    EXPECT_TRUE(filenamesFormFolder->empty());

    FileInterface::Managment::createFile(testPath + "3mwoznia_mnurzyn");
    FileInterface::Managment::createFile(testPath + "0tkogu_mnurzyn");
    filenamesFormFolder = FileInterface::Accesor::getFilenamesFromFolder(testPath);

    EXPECT_EQ(filenamesFormFolder->at(0), "0tkogu_mnurzyn");
    EXPECT_EQ(filenamesFormFolder->at(1), "3mwoznia_mnurzyn");

    FileInterface::Managment::removeFile(testPath + "3mwoznia_mnurzyn");
    FileInterface::Managment::removeFile(testPath + "0tkogu_mnurzyn");
}

TEST(FileHandlingTest, getFilenamesFromEmptyFolder)
{
    auto filenamesFormFolder = FileInterface::Accesor::getFilenamesFromFolder(testPath);

    EXPECT_TRUE(filenamesFormFolder->empty());
}

TEST(FileHandlingTest, addRow)
{
    FileInterface::Managment::createFile(testPath + filename);

    std::string row = "[mwoznia][3][33]";
    FileInterface::Modification::addRow(testPath + filename, row);

    std::fstream file(testPath + filename);
    std::string addedRow;
    getline(file, addedRow);

    EXPECT_EQ(row, addedRow);

    FileInterface::Managment::removeFile(testPath + filename);
}

TEST(FileHandlingTest, isRowNotOverwritte)
{
    FileInterface::Managment::createFile(testPath + filename);
    std::string row_1 = "[mwoznia][3][33]";
    std::string row_2 = "[mnurzyn][5][13]";

    FileInterface::Modification::addRow(testPath + filename, row_1);
    FileInterface::Modification::addRow(testPath + filename, row_2);

    std::fstream file(testPath + filename);
    std::string addedRow;

    getline(file, addedRow);
    EXPECT_EQ(row_1, addedRow);

    FileInterface::Managment::removeFile(testPath + filename);
}

TEST(FileHandlingTest, updateRow)
{
    FileInterface::Managment::createFile(testPath + filename);
    std::string row = "[mwoznia][3][33]";
    FileInterface::Modification::addRow(testPath + filename, row);
    std::string newRow = "[mwozniak][9][11]";
    FileInterface::Modification::updateRow(testPath + filename, newRow, "mwoznia");
    std::fstream file(testPath + filename);
    std::string updatedRow;

    getline(file, updatedRow);
    EXPECT_EQ(newRow, updatedRow);

    FileInterface::Managment::removeFile(testPath + filename);
}

TEST(FileHandlingTest, removeRow)
{
    FileInterface::Managment::createFile(testPath + filename);
    std::string row = "[mwoznia][3][33]";
    FileInterface::Modification::removeRow(testPath + filename, "mwoznia");

    std::fstream file(testPath + filename);
    std::string emptyRow;

    getline(file, emptyRow);
    EXPECT_TRUE(emptyRow.empty());

    FileInterface::Managment::removeFile(testPath + filename);
}

TEST(FileHandlingTest, isCorrectlyRowWasRemoved)
{
    FileInterface::Managment::createFile(testPath + filename);
    std::string firstRow = "[mwoznia][3][33]";
    std::string secondRow = "[mnurzyn][5][13]";

    FileInterface::Modification::addRow(testPath + filename, firstRow);
    FileInterface::Modification::addRow(testPath + filename, secondRow);

    FileInterface::Modification::removeRow(testPath + filename, "mwoznia");

    std::fstream file(testPath + filename);
    std::string row;
    getline(file, row);

    EXPECT_EQ(secondRow, row);

    FileInterface::Managment::removeFile(testPath + filename);
}

TEST(FileHandlingTest, updateRowField)
{
    FileInterface::Managment::createFile(testPath + filename);
    std::string firstRow = "[mwoznia][3][33]";
    FileInterface::Modification::addRow(testPath + filename, firstRow);

    FileInterface::Modification::updateRowField(testPath + filename, "mwoznia", "77", 1);

    std::fstream file(testPath + filename);
    std::string updatedRow;
    getline(file, updatedRow);

    EXPECT_EQ("[mwoznia][77][33]", updatedRow);

    FileInterface::Managment::removeFile(testPath + filename);
}

TEST(FileHandlingTest, getRowTest)
{
    FileInterface::Managment::createFile(testPath + filename);
    auto downloadedRow = FileInterface::Accesor::getRow(testPath + filename, "mwoznia");
    EXPECT_EQ(nullptr, downloadedRow);

    std::string addedRow_1 = "[mwoznia][3][33]";
    std::string addedRow_2 = "[tomek][13][33]";
    FileInterface::Modification::addRow(testPath + filename, addedRow_1);
    FileInterface::Modification::addRow(testPath + filename, addedRow_2);

    downloadedRow = FileInterface::Accesor::getRow(testPath + filename, "mwoznia");
    EXPECT_EQ("[mwoznia][3][33]", *downloadedRow);

    FileInterface::Managment::removeFile(testPath + filename);
}

TEST(FileHandlingTest, updateFlagsInFile)
{
    FileInterface::Managment::createFile(testPath + filename);
    std::string firstRow = "[mwoznia][3][33]";
    std::string secondRow = "[tomek][3][2]";
    std::string thirdRow = "[mwoznia][3][3]";
    FileInterface::Modification::addRow(testPath + filename, firstRow);
    FileInterface::Modification::addRow(testPath + filename, secondRow);
    FileInterface::Modification::addRow(testPath + filename, thirdRow);

    FileInterface::Modification::updateFlagsInFile(testPath + filename, "3", "10");

    std::array<std::string, 3> expectedRow {{"[mwoznia][10][33]", "[tomek][10][2]", "[mwoznia][10][3]"}};
    std::fstream file(testPath + filename);
    std::string updatedRow;

    for (int i = 0; i < 3; i++)
    {
        getline(file, updatedRow);
        EXPECT_EQ(expectedRow.at(i), updatedRow);
    }

    FileInterface::Managment::removeFile(testPath + filename);
}
