#include <vector>
#include <tuple>
#include <algorithm>
#include <string>

struct CProduct {
    std::string m_sKey;
    std::string m_sName;
    std::string m_sVersion;
};

typedef std::pair<std::string, std::tuple<CProduct, bool>> PAIR_PATHSTRING_PRODUCT;
typedef std::vector<PAIR_PATHSTRING_PRODUCT> VEC_PAIR_PATHSTRING_PRODUCT;

void AddDirectoryPath(VEC_PAIR_PATHSTRING_PRODUCT& vecProductPaths, const CProduct& product, const std::string& sPath, bool bEnterPlaceHolderOnExisting = true)
{
    //
    // Locate an existing path entry.
    //
    auto iter = std::find_if(vecProductPaths.begin(), vecProductPaths.end(), [&sPath](const PAIR_PATHSTRING_PRODUCT& entry) {
            //
            // Should be case insensitive compare for Windows...
            //
            return entry.first == sPath;
    });
    
    if(iter != vecProductPaths.end())
    {
        // Sigh... going for c++11 here, otherwise:
        // auto [foundProduct, bMultipleProducts] = iter->second;
        auto foundProduct = std::get<0>(iter->second);
        
        //
        // Don't add existing product
        //
        if(foundProduct.m_sKey == product.m_sKey)
        {
            printf("Already have product path for key '%s'\n", product.m_sKey.c_str());
            return;
        }
        
        //
        // Replace existing product; alternate version could keep a list of existing
        // products with the same path
        //
        // Using "dummy" product in this sample case when bEnterPlaceHolderOnExisting is true.
        //
        *iter = std::make_pair(sPath, std::make_tuple(bEnterPlaceHolderOnExisting ? CProduct() : product, true));
        return;
    }
    
    vecProductPaths.push_back(std::make_pair(sPath, std::make_tuple(product, false)));
}

void SortProductPaths(VEC_PAIR_PATHSTRING_PRODUCT& vecProductPaths)
{
    std::sort(vecProductPaths.begin(), vecProductPaths.end(), [](const PAIR_PATHSTRING_PRODUCT& a, const PAIR_PATHSTRING_PRODUCT& b) {
        //
        // Sort by the longest path key.
        //
        return a.first.length() > b.first.length();
    });
}

bool PathStartsWith(const std::string& a, const std::string& b)
{
    auto nPos = a.find(b);
    if(nPos == 0)
    {
        //
        // Full match
        //
        if(a.length() == b.length())
            return true;
        
        //
        // Match path if we ended the compare on a path unit (ending at '\'). Account for whether the path
        // prefix value has a trailing slash.
        //
        std::string::size_type nLastSlashCompare = b.length() - 1;
        if(b.at(nLastSlashCompare) != '\\')
            nLastSlashCompare++;
        
        if(a.length() < nLastSlashCompare)
            return false;
        if(a.at(nLastSlashCompare) == '\\')
            return true;
    }
    
    return false;
}

bool FindProductPath(const VEC_PAIR_PATHSTRING_PRODUCT& vecProductPaths, const std::string& sPath, std::tuple<CProduct, bool>& foundEntry)
{
    auto iter = std::find_if(vecProductPaths.begin(), vecProductPaths.end(), [&sPath](const PAIR_PATHSTRING_PRODUCT& entry) {
        //
        // Should be case insensitive for Windows...
        //
        return PathStartsWith(sPath, entry.first);
    });
    
    if(iter == vecProductPaths.end())
        return false;
    
    foundEntry = iter->second;
    
    return true;
}

void TestFind(const VEC_PAIR_PATHSTRING_PRODUCT& vecProductPaths, const std::string& sPath)
{
    std::tuple<CProduct, bool> foundEntry;
    auto bFound = FindProductPath(vecProductPaths, sPath, foundEntry);
    
    if(bFound)
        printf("Found: %s, has multiple: %s\n", std::get<0>(foundEntry).m_sKey.c_str(), std::get<1>(foundEntry) ? "true" : "false");
    else
        printf("Nothing found for '%s'\n", sPath.c_str());
}

int main(int argc, const char * argv[]) {
    VEC_PAIR_PATHSTRING_PRODUCT vecProductPaths;
    
    AddDirectoryPath(vecProductPaths, CProduct{"AAAA", "Product 1", "1.00.0000"}, "C:\\Program Files");
    AddDirectoryPath(vecProductPaths, CProduct{"BBBB", "Another Product", "2.1.1"}, "C:\\RootFolder\\TestProduct");
    AddDirectoryPath(vecProductPaths, CProduct{"CCCC", "Test 3", "3.0.11"}, "C:\\Program Files\\My Company\\My Product\\");
    AddDirectoryPath(vecProductPaths, CProduct{"CCCC", "Test 3", "3.0.11"}, "C:\\Program Files\\My Company\\My Product\\test1.exe");
    AddDirectoryPath(vecProductPaths, CProduct{"DDDD", "Product 4", "4.0.0"}, "C:\\Program Files");
    
    SortProductPaths(vecProductPaths);
    
    for(const auto& entry : vecProductPaths)
    {
        auto multiples = std::get<1>(entry.second);
        printf("Entry path: %s, has multiple: %s\n", entry.first.c_str(), multiples ? "true" : "false");
    }
    
    TestFind(vecProductPaths, "C:\\NotFound\\test0.exe");
    TestFind(vecProductPaths, "C:\\Program Files\\My Company\\My Product\\test1.exe");
    TestFind(vecProductPaths, "C:\\Program Files\\My Company\\test_b.exe");
    TestFind(vecProductPaths, "C:\\Program Files\\My Company\\My Product\\bin\\test_a.exe");
    TestFind(vecProductPaths, "C:\\RootFolder\\TestProduct\\bin\\test.exe");
    TestFind(vecProductPaths, "C:\\Program Files\\Hello World\\bin\\test2.exe");
    TestFind(vecProductPaths, "C:\\Program Files\\unknown path\\test3.exe");

    return 0;
}
