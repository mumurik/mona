#include "ISO9660FileSystem.h"
#include "sys/error.h"

using namespace std;
using namespace iso9660;

extern string upperCase(const string& s);

ISO9660FileSystem::ISO9660FileSystem(IStorageDevice* drive, VnodeManager* vmanager) : drive_(drive), vmanager_(vmanager)
{
}

ISO9660FileSystem::~ISO9660FileSystem()
{
    deleteEntry(rootDirectory_);
}

/*----------------------------------------------------------------------
    public interfaces
----------------------------------------------------------------------*/
int ISO9660FileSystem::initialize()
{
    if (readVolumeDescriptor() != MONA_SUCCESS)
    {
        _printf("read volume descriptor error%s %s:%d\n", __func__, __FILE__, __LINE__);
        return MONA_FAILURE;
    }

    if (setDirectoryCache() != MONA_SUCCESS)
    {
        return MONA_FAILURE;
    }

    root_ = vmanager_->alloc();
    root_->fnode  = rootDirectory_;
    root_->fs     = this;
    root_->type = Vnode::DIRECTORY;
    return MONA_SUCCESS;
}

int ISO9660FileSystem::lookup(Vnode* diretory, const string& file, Vnode** found, int type)
{
    if (diretory->type != Vnode::DIRECTORY) return MONA_ERROR_INVALID_ARGUMENTS;
    Vnode* v = vmanager_->cacher()->lookup(diretory, file);
    if (v != NULL && v->type == type)
    {
        *found = v;
        return MONA_SUCCESS;
    }
    Entry* directoryEntry = (Entry*)diretory->fnode;
    Entry* target = NULL;

    if (type == Vnode::REGULAR)
    {
        target = lookupFile(directoryEntry, file);
    }
    else
    {
        target = lookupDirectory(directoryEntry, file);
    }
    if (target == NULL) return MONA_ERROR_ENTRY_NOT_FOUND;
    Vnode* newVnode = vmanager_->alloc();
    newVnode->fnode  = target;
    newVnode->type = type;
    newVnode->fs = this;
    vmanager_->cacher()->add(diretory, file, newVnode);
    *found = newVnode;
    return MONA_SUCCESS;
}

int ISO9660FileSystem::open(Vnode* file, int mode)
{
    return MONA_SUCCESS;
}

int ISO9660FileSystem::create(Vnode* dir, const string& file)
{
    return MONA_FAILURE;
}

int ISO9660FileSystem::write(Vnode* file, struct io::Context* context)
{
    return MONA_ERROR_ON_ACCESS;
}

int ISO9660FileSystem::read(Vnode* file, struct io::Context* context)
{
    Entry* fileEntry = (Entry*)file->fnode;
    uint32_t offset = context->offset;
    uint32_t readSize = context->size;
    uint32_t rest = fileEntry->attribute.size - offset;
    if (rest == 0) return MONA_FAILURE;

    if (rest < readSize)
    {
        readSize = rest;
    }

    int lba = fileEntry->attribute.extent + offset / SECTOR_SIZE;
    int sectorCount = (offset + readSize + SECTOR_SIZE - 1) / SECTOR_SIZE - offset / SECTOR_SIZE;
    uint32_t sectorSize = sectorCount * SECTOR_SIZE;

#if 0
    byte* temp = new byte[sectorSize];
    if (temp == NULL) return MONA_FAILURE;

    bool readResult = drive_->read(lba, temp, sectorSize) == 0;
    if (!readResult)
    {
        delete[] temp;
        return MONA_FAILURE;
    }

    context->memory = monapi_cmemoryinfo_new();

    if (monapi_cmemoryinfo_create(context->memory, readSize, MONAPI_FALSE) != M_OK)
    {
        monapi_cmemoryinfo_delete(context->memory);
        delete[] temp;
        return MONA_ERROR_MEMORY_NOT_ENOUGH;
    }
    memcpy(context->memory->Data, temp + offset -(lba - fileEntry->attribute.extent) * SECTOR_SIZE, readSize);
    delete[] temp;
    context->resultSize = readSize;
    return MONA_SUCCESS;
#else
    int dataOffset = offset - (lba - fileEntry->attribute.extent) * SECTOR_SIZE;
    context->memory = monapi_cmemoryinfo_new();
    if (monapi_cmemoryinfo_create(context->memory, readSize, MONAPI_FALSE) != M_OK)
    {
        monapi_cmemoryinfo_delete(context->memory);
        return MONA_ERROR_MEMORY_NOT_ENOUGH;
    }
    // by junjunn
    if (0 == dataOffset)
    {
// by higepon
//        bool readResult = drive_->read(lba, context->memory->Data, sectorSize) == 0;
        bool readResult = drive_->read(lba, context->memory->Data, readSize) == 0;
        if (!readResult)
        {
            return MONA_FAILURE;
        }
    }
    else
    {
        uint8_t* temp = new uint8_t[sectorSize];
        if (temp == NULL) return MONA_FAILURE;
        bool readResult = drive_->read(lba, temp, sectorSize) == 0;
        if (!readResult)
        {
            delete temp;
            return MONA_FAILURE;
        }

        memcpy(context->memory->Data, temp + dataOffset, readSize);
        delete[] temp;
    }
    context->resultSize = readSize;
    return MONA_SUCCESS;
#endif
}

int ISO9660FileSystem::seek(Vnode* file, uint32_t offset, uint32_t origin)
{
    return MONA_SUCCESS;
}

int ISO9660FileSystem::close(Vnode* file)
{
    return MONA_SUCCESS;
}

int ISO9660FileSystem::stat(Vnode* file, Stat* st)
{
    Entry* entry = (Entry*)file->fnode;
    st->size = entry->attribute.size;
    return MONA_SUCCESS;
}

Vnode* ISO9660FileSystem::getRoot() const
{
    return root_;
}

int ISO9660FileSystem::readdir(Vnode* dir, monapi_cmemoryinfo** entries)
{
    Entry* directory = (Entry*)dir->fnode;
    setDetailInformation(directory);
    uint32_t readSize = ((uint32_t)((directory->attribute.size + SECTOR_SIZE - 1) / SECTOR_SIZE)) * SECTOR_SIZE;
    uint8_t* buffer = readdirToBuffer(directory, readSize);
    if (buffer == NULL) return MONA_ERROR_MEMORY_NOT_ENOUGH;
    EntryList entryList;
    for (uint32_t position = 0 ; position < readSize;)
    {
        DirectoryEntry* iEntry = (DirectoryEntry*)((uint32_t)buffer + position);

        if (iEntry->length == 0)
        {
            // check next sector
            position = ((position + SECTOR_SIZE - 1) / SECTOR_SIZE) * SECTOR_SIZE;
            continue;
        }
        Entry* entry = setupEntry(iEntry);
        entryList.push_back(entry);
        position += iEntry->length;
    }
    delete[] buffer;
    monapi_cmemoryinfo* ret = monapi_cmemoryinfo_new();
    int size = entryList.size();
    if (monapi_cmemoryinfo_create(ret, sizeof(int) + size * sizeof(monapi_directoryinfo), MONAPI_FALSE) != M_OK)
    {
        monapi_cmemoryinfo_delete(ret);
        return MONA_ERROR_MEMORY_NOT_ENOUGH;
    }
    memcpy(ret->Data, &size, sizeof(int));
    monapi_directoryinfo* p = (monapi_directoryinfo*)&ret->Data[sizeof(int)];
    for (EntryList::iterator i = entryList.begin(); i != entryList.end(); ++i)
    {
        monapi_directoryinfo di;

        di.size = (*i)->attribute.size;
        strcpy(di.name, (*i)->name.c_str());
        di.attr = (*i)->isDirectory ? ATTRIBUTE_DIRECTORY : 0;
        *p = di;
        p++;
        delete (*i);
    }
    *entries = ret;

    return MONA_SUCCESS;
}

/*----------------------------------------------------------------------
    private functions
----------------------------------------------------------------------*/
// you should delete return value
uint8_t* ISO9660FileSystem::readdirToBuffer(Entry* directory, uint32_t readSize)
{
    uint8_t* buffer = new uint8_t[readSize];

    if (buffer == NULL)
    {
        return NULL;
    }
    bool readResult = drive_->read(directory->attribute.extent, buffer, readSize) == 0;

    if (!readResult)
    {
        delete buffer;
        return NULL;
    }
    return buffer;
}

// you should delete return value
Entry* ISO9660FileSystem::setupEntry(DirectoryEntry* from)
{
    Entry* entry = new Entry;
    entry->isDirectory = from->directory == 1;
    if (from->length == 1 && from->name[0] == 0x00)
    {
        entry->name = ".";
    }
    else if (from->length == 1 && from->name[0] == 0x01)
    {
        entry->name = "..";
    }
    else
    {
        entry->name = getProperName(string((const char*)from->name, from->name_len));
    }
    return entry;
}

int ISO9660FileSystem::readVolumeDescriptor()
{
    int i;
    bool primaryVolumeDescriptorFound = false;

    BaseVolumeDescriptor* descriptor = new BaseVolumeDescriptor;
    if (descriptor == NULL)
    {
        _printf("BaseVolumeDescriptor allocate error%s %s:%d\n", __func__, __FILE__, __LINE__);
        return MONA_FAILURE;
    }
    for (i = 16; i < 100; i++)
    {
        // read
        bool readResult = drive_->read(i, descriptor, SECTOR_SIZE) == 0;
        if (!readResult)
        {
            delete descriptor;
            _printf("device read error%s %s:%d\n", __func__, __FILE__, __LINE__);
            return MONA_FAILURE;
        }
        // read primary descriptor
        if (descriptor->type == ISO_PRIMARY_VOLUME_DESCRIPTOR && strncmp("CD001", descriptor->id, 5) == 0)
        {
            PrimaryVolumeDescriptor* p = (PrimaryVolumeDescriptor*)(descriptor);

            // keep primary descriptor
            pdescriptor_ = *p;
            primaryVolumeDescriptorFound = true;
        }

        // end
        if (descriptor->type == ISO_END_VOLUME_DESCRIPTOR)
        {
            break;
        }
    }
    // invalid
    if (i == 100 || !primaryVolumeDescriptorFound)
    {
        _printf("BaseVolumeDescriptor allocate error%s %s:%d\n", __func__, __FILE__, __LINE__);
        return MONA_FAILURE;
    }

    delete descriptor;
    return MONA_SUCCESS;
}

uint8_t* ISO9660FileSystem::readPathTableIntoBuffer()
{
    uint32_t readSize = ((uint32_t)((pdescriptor_.path_table_size_l + SECTOR_SIZE - 1) / SECTOR_SIZE)) * SECTOR_SIZE;
    uint8_t* buffer = new uint8_t[readSize];

    if (buffer == NULL)
    {
        return NULL;
    }

    bool readResult = drive_->read(pdescriptor_.type_l_path_table, buffer, readSize) == 0;

    return readResult ? buffer : NULL;
}

int ISO9660FileSystem::setDirectoryCache()
{
     uint8_t* buffer;

    // read path table
    if ((buffer = readPathTableIntoBuffer()) == NULL)
    {
        return MONA_FAILURE;
    }

    // create DirectoryEntries from path table
    EntryList directoryList;

    createDirectoryListFromPathTable(&directoryList, buffer);
    delete[] buffer;

    // directory not found
    if (directoryList.size() == 0)
    {
        return MONA_FAILURE;
    }

    // set up root direcotyr
    rootDirectory_ = directoryList[0];

    DirectoryEntry* rootEntry = (DirectoryEntry*)((pdescriptor_.root_directory_record));
    setDetailInformation(rootDirectory_, rootEntry);

    // set children
    for (uint32_t i = 0; i < directoryList.size(); i++)
    {
        setDirectoryRelation(&directoryList, directoryList[i]);
    }

    // parent of root is root
    directoryList[0]->parent = directoryList[0];

    return MONA_SUCCESS;
}

void ISO9660FileSystem::createDirectoryListFromPathTable(EntryList* list, uint8_t* buffer)
{
    for (uint32_t id = 1, position = 0; position < pdescriptor_.path_table_size_l; id++)
    {
        Entry* entry = new Entry;

        PathTableEntry* pathEntry = (PathTableEntry*)(buffer + position);
        entry->attribute.id       = id;
        entry->attribute.extent   = pathEntry->extent;
        entry->attribute.parentID = pathEntry->parentDirectory;

        if (pathEntry->length == 1 && pathEntry->name[0] == 0x00)
        {
            entry->name = ".";
        }
        else if (pathEntry->length == 1 && pathEntry->name[0] == 0x01)
        {
            entry->name = "..";
        }
        else
        {
            entry->name = upperCase(string((const char*)pathEntry->name, pathEntry->length));
        }
        list->push_back(entry);

        /* next path table entry */
        position += pathEntry->length + sizeof(PathTableEntry) + (pathEntry->length % 2 ? 1 : 0);
    }
}

void ISO9660FileSystem::setDetailInformation(Entry* to, DirectoryEntry* from)
{
    FileDate* createDate = &(to->createDate);

    string tmp((const char*)from->name, from->name_len);
    to->name = getProperName(tmp);

    to->attribute.extent= from->extent_l;
    to->attribute.size  = from->size_l;
    createDate->setYear(from->date[0] + 1900);
    createDate->setMonth(from->date[1]);
    createDate->setDay(from->date[2]);
    createDate->setHour(from->date[3]);
    createDate->setMinute(from->date[4]);
    createDate->setSecond(from->date[5]);
    to->hasDetail = true;
}

string ISO9660FileSystem::getProperName(const string& name)
{
    string result = name;

    if (result[0] == 0x00)
    {
        result = ".";
    }
    else if (result[0] == 0x01)
    {
        result = "..";
    }

    if (name.find(';') != string::npos)
    {
        result = result.substr(0, result.find(";"));
    }

    if (result[result.size() - 1] == '.' && result != "." && result != "..")
    {
        result = result.substr(0, result.size() - 1);
    }
    return upperCase(result);
}

void ISO9660FileSystem::setDirectoryRelation(EntryList* list, Entry* directory)
{
    uint32_t self = directory->attribute.id;

    for (uint32_t i = 0; i < list->size(); i++)
    {
        Entry* entry = list->at(i);

        if (entry->attribute.parentID != self) continue;
        if (i == 0 && entry->attribute.parentID == 1) continue;

        directory->children.push_back(entry);
        entry->parent = directory;
    }
}

void ISO9660FileSystem::deleteEntry(Entry* entry)
{
    if (entry == NULL) return;
    EntryList* children = &(entry->children);
    for (EntryList::iterator i = children->begin(); i != children->end(); ++i )
    {
        deleteEntry(*i);
    }
    delete entry;
}

void ISO9660FileSystem::split(string str, char ch, vector<string>& v)
{
    uint32_t index = 0;
    uint32_t next = 0;
    while ((index = str.find_first_of(ch, next)) != string::npos)
    {
        v.push_back(string(str.begin() + next, str.begin() + index));
        next = index + 1;
    }
    v.push_back(string(str.begin() + next, str.end()));
}

Entry* ISO9660FileSystem::lookupDirectory(Entry* root, const string& path)
{
    bool found;
    vector<string> elements;
    split(path, '/', elements);

    for (vector<string>::iterator element = elements.begin(); element != elements.end(); ++element)
    {
        if (*element == ".")
        {
            continue;
        }
        else if (*element == "..")
        {
            root = root->parent;
            continue;
        }
        else if (*element == "")// || *element == NULL)
        {
            continue;
        }

        found = false;
        for (EntryList::iterator i = root->children.begin(); i != root->children.end(); ++i)
        {
            if ((*i)->name == *element)
            {
                root = *i;
                found = true;
                break;
            }
        }
        if (!found) return NULL;
    }
    return root;
}

// return value should be delete, when close
Entry* ISO9660FileSystem::lookupFile(Entry* directory, const string& fileName)
{
    setDetailInformation(directory);
    uint32_t readSize = ((uint32_t)((directory->attribute.size + SECTOR_SIZE - 1) / SECTOR_SIZE)) * SECTOR_SIZE;
    uint8_t* buffer = new uint8_t[readSize];

    if (buffer == NULL)
    {
        return NULL;
    }

    bool readResult = drive_->read(directory->attribute.extent, buffer, readSize) == 0;

    if (!readResult)
    {
        delete buffer;
        return NULL;
    }

    for (uint32_t position = 0; position < readSize;)
    {
        DirectoryEntry* iEntry = (DirectoryEntry*)(buffer + position);

        if (iEntry->length == 0)
        {
            // check next sector
            position = ((position + SECTOR_SIZE - 1) / SECTOR_SIZE) * SECTOR_SIZE;
            continue;
        }

        string name = getProperName(string(iEntry->name, iEntry->name_len));
        if (iEntry->directory == 0 && fileName == upperCase(name))
        {
            Entry* foundFile = new Entry;

            setDetailInformation(foundFile, iEntry);

            delete[] buffer;
            return foundFile;
        }

        position += iEntry->length;
        iEntry = (DirectoryEntry*)(buffer + position);
    }
    delete[] buffer;
    return NULL;
}

bool ISO9660FileSystem::setDetailInformation(Entry* entry)
{
    if (entry->hasDetail) return true;

    if (!(entry->parent->hasDetail))
    {
        setDetailInformation(entry->parent);
    }

    uint32_t readSize = ((uint32_t)((entry->parent->attribute.size + SECTOR_SIZE - 1) / SECTOR_SIZE)) * SECTOR_SIZE;
    uint8_t* buffer = new uint8_t[readSize];

    if (buffer == NULL)
    {
        return false;
    }

    bool readResult = drive_->read(entry->parent->attribute.extent, buffer, readSize) == 0;

    if (!readResult)
    {
        delete[] buffer;
        return false;
    }

    EntryList* children = &(entry->parent->children);

    for (uint32_t position = 0; position < readSize;)
    {
        DirectoryEntry* iEntry = (DirectoryEntry*)(buffer + position);
        string name = string(iEntry->name, iEntry->name_len);

        if (iEntry->length == 0)
        {
            position = ((position + SECTOR_SIZE - 1) / SECTOR_SIZE) * SECTOR_SIZE;
            continue;
        }
        for (EntryList::iterator i = children->begin(); i != children->end(); ++i)
        {
            Entry* child = *i;
            if (upperCase(name) != child->name) continue;

            setDetailInformation(child, iEntry);
        }

        position += iEntry->length;
    }
    delete[] buffer;
    return true;
}

void ISO9660FileSystem::destroyVnode(Vnode* vnode)
{
    iso9660::Entry* entry = (iso9660::Entry*)vnode->fnode;
    if (vnode->type != Vnode::DIRECTORY) delete entry; // directory is deleted on destructor
    delete vnode;
}
