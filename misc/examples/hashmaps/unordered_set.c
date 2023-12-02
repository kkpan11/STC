// https://iq.opengenus.org/containers-cpp-stl/
// C program to demonstrate various function of stc hset
#define i_implement
#include "stc/cstr.h"
#define i_key_str
#include "stc/hset.h"

int main(void)
{
    // declaring set for storing string data-type
    hset_str stringSet = {0};
    c_defer(
        hset_str_drop(&stringSet)
    ){
        // inserting various string, same string will be stored
        // once in set
        hset_str_emplace(&stringSet, "code");
        hset_str_emplace(&stringSet, "in");
        hset_str_emplace(&stringSet, "C");
        hset_str_emplace(&stringSet, "is");
        hset_str_emplace(&stringSet, "fast");

        const char* key = "slow";

        //     find returns end iterator if key is not found,
        //  else it returns iterator to that key

        if (hset_str_find(&stringSet, key).ref == NULL)
            printf("\"%s\" not found\n", key);
        else
            printf("Found \"%s\"\n", key);

        key = "C";
        if (!hset_str_contains(&stringSet, key))
            printf("\"%s\" not found\n", key);
        else
            printf("Found \"%s\"\n", key);

        // now iterating over whole set and printing its
        // content
        printf("All elements :\n");
        c_foreach (itr, hset_str, stringSet)
            printf("%s\n", cstr_str(itr.ref));
    }
}
