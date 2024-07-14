/**
 * @file main.cpp
 * @author Harsh Raj
 * @date 13 July, 2024
 * @brief A dummy implementation of file manager,
 * with a file storage in form of a n-ary tree like data structure.
 */

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <memory>

class File;
class Folder;
class FileStorage;
class FileManager;

class File
{
private:
    friend class FileManager;

    struct Metadata
    {
        std::size_t fileSize_;
        std::string fullPath_;
        std::string fileExtension_;
        Metadata(std::size_t size, std::string fullPath, std::string fileExtension) : fileSize_{size}, fullPath_{fullPath}, fileExtension_{fileExtension} {}
    };
    Metadata metadata_;
    std::string content_;

    File(const std::string fullPath, const std::string fileExtension, const std::string content) : metadata_{content.size(), fullPath, fileExtension}, content_{content} {};

    void updateContent(const std::string &newFileContent)
    {
        content_ = newFileContent;
        metadata_.fileSize_ = newFileContent.size();
    }

    void printContents() const noexcept
    {
        // Print metadata
        std::cout << "Metadata: ";
        std::cout << "Full Path: " << metadata_.fullPath_ << ", ";
        std::cout << "File Size: " << metadata_.fileSize_ << ", ";
        std::cout << "File Extension: " << metadata_.fileExtension_ << std::endl;

        // Print Contents
        std::cout << "Contents: " << content_ << std::endl;
    }
};

class Folder
{
private:
    friend class FileManager;
    friend class FileStorage;

    struct Metadata
    {
        int foldersCount_;
        int filesCount_;
        std::string fullPath_;
        Metadata(int foldersCount, int filesCount, std::string fullPath) : foldersCount_{foldersCount}, filesCount_{filesCount}, fullPath_{fullPath} {}
    };
    Metadata metadata_;
    Folder *parentFolder_;
    std::unordered_map<std::string, Folder *> folders_;
    std::unordered_map<std::string, File *> files_;

    Folder(std::string fullPath, Folder *parentFolder) : metadata_{0, 0, fullPath}, parentFolder_{parentFolder}
    {
        if (parentFolder != nullptr)
            folders_[".."] = parentFolder;
    }

    void addFolder(std::string newFolderName, Folder *newFolderPointer) noexcept
    {
        folders_[newFolderName] = newFolderPointer;
        metadata_.foldersCount_++;
    }

    void addFile(std::string newFileName, File *newFilePointer) noexcept
    {
        files_[newFileName] = newFilePointer;
        metadata_.filesCount_++;
    }

    void removeFolder(std::string folderName) noexcept
    {
        delete folders_[folderName];
        folders_.erase(folderName);
        metadata_.foldersCount_--;
    }

    void removeFile(std::string fileName) noexcept
    {
        delete files_[fileName];
        files_.erase(fileName);
        metadata_.filesCount_--;
    }

    void printContents() const noexcept
    {
        // Print metadata
        std::cout << "Metadata: ";
        std::cout << "Full Path: " << metadata_.fullPath_ << ", ";
        std::cout << "No. of folders: " << metadata_.foldersCount_ << ", ";
        std::cout << "No. of files: " << metadata_.filesCount_ << std::endl;

        // Print folders
        std::cout << "Folders: ";
        for (auto curFolder : folders_)
            std::cout << curFolder.first << ", ";
        std::cout << std::endl;

        // Print files
        std::cout << "Files: ";
        for (auto curFiles : files_)
            std::cout << curFiles.first << ", ";
        std::cout << std::endl;
    }

public:
    ~Folder() noexcept
    {
        // deleting all child folders too
        for (auto curFolder : folders_)
        {
            if (curFolder.first == "..")
                continue;
            delete curFolder.second;
        }
        // deleting all child files too
        for (auto curFile : files_)
        {
            delete curFile.second;
        }
    }
};

/**
 * @class FileStorage
 * @brief Simulates a n-ary tree like file storage,
 * think of this like a file partition or a disc on your computer.
 */
class FileStorage
{
private:
    Folder *rootFolder;

public:
    FileStorage()
    {
        // Initialize a root folder for this storage here, this value will be used by FileManager objects
        rootFolder = new Folder("/", nullptr);
    }

    /**
     * @brief gets the root folder pointer
     * @return pointer to the root folder of type Folder *
     */
    Folder *getRootFolder() const
    {
        return rootFolder;
    }

    /**
     * @brief deletes the whole tree that we created as the storage,
     * including all the childeren too by calling destructor of the root folder
     */
    ~FileStorage() noexcept
    {
        delete rootFolder;
        std::cout << "\n=====\n"
                  << "Storage Deleted" << std::endl;
    }
};

/**
 * @class FileManager
 * @brief Takes a FileStorage object and helps you do all the CRUD operations on that storage
 */
class FileManager
{
    FileStorage *fileStorage_;
    Folder *currentDirPointer_;
    std::string currentDirPath_;

    static std::string getFileExtension(const std::string &fileName)
    {
        int fileNameSize = fileName.size();
        for (int i = fileNameSize - 1; i >= 0; i--)
        {
            // going to the last `.`
            if (fileName[i] == '.')
            {
                // updating extension and returning it
                std::string extension{""};
                for (int j = i + 1; j < fileNameSize; j++)
                {
                    extension.push_back(fileName[i]);
                }
                return extension;
            }
        }
        // no extension found
        return "";
    }

    /**
     * @brief split a filePath into a vector from every "/" occurance in it
     * @param filePath the path to split
     * @throws std::runtime_error if filePath has preceeding or adjacent "/"
     */
    static std::vector<std::string> splitFilePath(const std::string &filePath)
    {
        std::vector<std::string> splits;
        std::size_t index = 0;
        std::size_t filePath_size = filePath.size();

        if (filePath_size == 0)
            return splits;

        if (filePath[index] == '/')
            throw std::runtime_error("Preceeding \"/\" not allowed in filePath");

        while (index < filePath_size)
        {
            std::string curSplit;
            while (index < filePath_size && filePath[index] != '/')
            {
                curSplit.push_back(filePath[index]);
                index++;
            }
            if (index + 1 < filePath_size && filePath[index + 1] == '/')
            {
                throw std::runtime_error("Adjacent \"/\" not allowed in filePath");
            }
            splits.push_back(curSplit);
            index++;
        }

        return splits;
    }

    /**
     * @brief checks if a file or folder name is valid or not
     * @param name string, the name of file or folder to be validated
     * @throws std::invalid_argument if name is invalid
     */
    static void throwIfNameInvalid(const std::string &name)
    {
        if (name.find('/') != std::string::npos)
            throw std::invalid_argument("File or folder names can't contain \"/\" in them");
    }

public:
    /**
     * @brief Create a FileManager object at the root folder
     * @param fileStorage pointer to an instance of a FileStorage object
     * that needs to be managed by the this object
     */
    FileManager(FileStorage *fileStorage) : fileStorage_{fileStorage}, currentDirPointer_{fileStorage->getRootFolder()}, currentDirPath_{"/"} {}

    /**
     * @brief Create a FileManager object at the specified folder
     * @param destinationFolder string denoting the location to jump to,
     * preceeding "/" are not allowed in destinationFolder;
     * use ".." to go to the parent folder
     * @param relative boolean telling is the path is relative to current folder
     * or is it an absolute path from the root folder
     * @throws std::runtime_error if destinationFolder can't be found,
     * folder isn't changed in case of this error
     * (this error is caught and handled internally)
     */
    void changeDirectory(std::string destinationFolder, bool relative)
    {
        if (currentDirPath_ == destinationFolder)
            return;

        Folder *tempDirPointer = currentDirPointer_;
        std::vector<std::string> destinationFolderSpilt;

        if (relative == false)
        {
            // returning to the root folder if path is absolute from the root folder
            tempDirPointer = fileStorage_->getRootFolder();
        }

        try
        {
            destinationFolderSpilt = splitFilePath(destinationFolder);

            for (std::string nextFolderName : destinationFolderSpilt)
            {
                if (tempDirPointer->folders_.count(nextFolderName) == 0)
                {
                    throw std::runtime_error("Destination folder can't be found");
                }
                tempDirPointer = tempDirPointer->folders_[nextFolderName];
            }

            // Updating the current instance's currentDir pointer & path
            // only after the destination folder reached without any errors
            currentDirPointer_ = tempDirPointer;
            currentDirPath_ = destinationFolder;
        }
        catch (const std::runtime_error &e)
        {
            std::cerr << "Couldn't change directory: " << e.what() << std::endl;
        }
    }

    /**
     * @brief prints the current working directory
     */
    void printWorkingDirectory() const noexcept
    {
        std::cout << "Current Working Directory: " << currentDirPath_ << std::endl;
    }

    // Adding CRUD functionalities below

    /**
     * @brief create a folder in current directory
     * @param folderName denoting the name of folder to be created
     * @throws std::invalid_argument if folderName isn't valid
     * (caught and handled internally)
     * @throws std::runtime_error if folderName already exists
     * (caught and handled internally)
     */
    void createFolder(std::string folderName)
    {
        try
        {
            throwIfNameInvalid(folderName);
            if (currentDirPointer_->folders_.count(folderName) != 0)
                throw std::runtime_error("Folder already exists");
            std::string newFolderPath = currentDirPath_ + "/" + folderName;
            Folder *newFolderPointer = new Folder(newFolderPath, currentDirPointer_);
            currentDirPointer_->addFolder(folderName, newFolderPointer);
        }
        catch (std::invalid_argument &e)
        {
            std::cerr << "Error while creating folder: " << e.what() << std::endl;
        }
    }

    /**
     * @brief create a file in current directory
     * @param fileName denoting the name of file to be created
     * @param fileContent denoting the content of the file
     * @throws std::invalid_argument if fileName isn't valid
     * (caught and handled internally)
     * @throws std::runtime_error if fileName already exists
     * (caught and handled internally)
     */
    void createFile(std::string fileName, std::string fileContent = "")
    {
        try
        {
            throwIfNameInvalid(fileName);
            if (currentDirPointer_->files_.count(fileName) != 0)
                throw std::runtime_error("File already exists");
            std::string newFilePath = currentDirPath_ + "/" + fileName;
            std::string extension = getFileExtension(fileName);
            File *newFilePointer = new File(newFilePath, extension, fileContent);
            currentDirPointer_->addFile(fileName, newFilePointer);
        }
        catch (std::invalid_argument &e)
        {
            std::cerr << "Error while creating file: " << e.what() << std::endl;
        }
    }

    /**
     * @brief update a file in current directory
     * @param fileName denoting the name of file to be updated
     * @param fileContent denoting the content of the file
     * @throws std::invalid_argument if fileName isn't valid
     * (caught and handled internally)
     * @throws std::runtime_error if fileName doesn't exist
     * (caught and handled internally)
     */
    void updateFile(std::string fileName, std::string fileContent)
    {
        try
        {
            throwIfNameInvalid(fileName);
            if (currentDirPointer_->files_.count(fileName) == 0)
                throw std::runtime_error("File doesn't exists");
            currentDirPointer_->files_[fileName]->updateContent(fileContent);
        }
        catch (std::invalid_argument &e)
        {
            std::cerr << "Error while updating file: " << e.what() << std::endl;
        }
    }

    /**
     * @brief print contents of the current folder
     */
    void printCurrentFolderContents() const noexcept
    {
        currentDirPointer_->printContents();
    }

    /**
     * @brief print contents of the current file
     */
    void printFileContents(std::string fileName) const noexcept
    {
        try
        {
            throwIfNameInvalid(fileName);
            if (currentDirPointer_->files_.count(fileName) == 0)
                throw std::runtime_error("File doesn't exists");
            currentDirPointer_->files_[fileName]->printContents();
        }
        catch (std::invalid_argument &e)
        {
            std::cerr << "Error while printing file: " << e.what() << std::endl;
        }
    }

    /**
     * @brief delete a folder in current directory
     * @param folderName denoting the name of folder to be deleted
     * @throws std::invalid_argument if folderName isn't valid
     * (caught and handled internally)
     * @throws std::runtime_error if folderName doesn't exist
     * (caught and handled internally)
     */
    void deleteFolder(std::string folderName)
    {
        try
        {
            throwIfNameInvalid(folderName);
            if (currentDirPointer_->folders_.count(folderName) == 0)
                throw std::runtime_error("Folder doesn't exists");
            currentDirPointer_->removeFolder(folderName);
        }
        catch (std::invalid_argument &e)
        {
            std::cerr << "Error while deleting folder: " << e.what() << std::endl;
        }
    }

    /**
     * @brief delete a file in current directory
     * @param folderName denoting the name of file to be deleted
     * @throws std::invalid_argument if fileName isn't valid
     * (caught and handled internally)
     * @throws std::runtime_error if fileName doesn't exist
     * (caught and handled internally)
     */
    void deleteFile(std::string fileName)
    {
        try
        {
            throwIfNameInvalid(fileName);
            if (currentDirPointer_->files_.count(fileName) == 0)
                throw std::runtime_error("File doesn't exists");
            currentDirPointer_->removeFile(fileName);
        }
        catch (std::invalid_argument &e)
        {
            std::cerr << "Error while deleting file: " << e.what() << std::endl;
        }
    }
};

int main()
{
    FileStorage *fileStorage = new FileStorage();
    FileManager *fileManager = new FileManager(fileStorage);

    fileManager->printCurrentFolderContents();
    fileManager->createFolder("aaa");
    fileManager->printCurrentFolderContents();
    fileManager->changeDirectory("aaa", true);
    fileManager->printCurrentFolderContents();
    fileManager->createFolder("bbb");
    fileManager->changeDirectory("aaa/bbb", false);
    fileManager->printWorkingDirectory();
    fileManager->changeDirectory("../..", true);
    fileManager->printWorkingDirectory();

    fileManager->printCurrentFolderContents();
    fileManager->createFile("yoyo", "huhu");
    fileManager->printCurrentFolderContents();

    fileManager->printFileContents("yoyo");
    fileManager->updateFile("yoyo", "huuuuuuuuuuuuuuuuuuuuu");
    fileManager->printFileContents("yoyo");

    fileManager->printCurrentFolderContents();
    fileManager->deleteFile("yoyo");
    fileManager->printCurrentFolderContents();

    fileManager->createFolder("ccc");
    fileManager->deleteFolder("ccc");

    delete fileManager;
    delete fileStorage;
}
