/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - util.c                                                  *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Hacktarux                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Provides common utilities to the rest of the code:
 *  -String functions
 *  -Doubly-linked list
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>

#include "rom.h"
#include "util.h"
#include "osal/files.h"
#include "osal/preproc.h"

/** read_from_file
 *    opens a file and reads the specified number of bytes.
 *    returns zero on success, nonzero on failure
 */
int read_from_file(const char *filename, void *data, size_t size)
{
    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        return -1;
    }

    if (fread(data, 1, size, f) != size)
    {
        fclose(f);
        return -1;
    }

    fclose(f);
    return 0;
}

/** write_to_file
 *    opens a file and writes the specified number of bytes.
 *    returns zero on sucess, nonzero on failure
 */ 
int write_to_file(const char *filename, const void *data, size_t size)
{
    FILE *f = fopen(filename, "wb");
    if (f == NULL)
    {
        return -1;
    }

    if (fwrite(data, 1, size, f) != size)
    {
        fclose(f);
        return -1;
    }

    fclose(f);
    return 0;
}

/** trim
 *    Removes leading and trailing whitespace from str. Function modifies str
 *    and also returns modified string.
 */
char *trim(char *str)
{
    unsigned int i;
    char *p = str;

    while (isspace(*p))
        p++;

    if (str != p)
        {
        for (i = 0; i <= strlen(p); ++i)
            str[i]=p[i];
        }

    p = str + strlen(str) - 1;
    if (p > str)
    {
        while (isspace(*p))
            p--;
        p[1] = '\0';
    }

    return str;
}

/** list_empty
 *    Returns 1 if list is empty, else 0.
 */
static int list_empty(list_t list)
{
    return list == NULL;
}

/** linked list functions **/

/** list_prepend
 *    Allocates a new list node, attaches it to the beginning of list and sets the
 *    node data pointer to data.
 *    Returns - the new list node.
 */
list_node_t *list_prepend(list_t *list, void *data)
{
    list_node_t *new_node,
                *first_node;

    if(list_empty(*list))
    {
        (*list) = (list_t) malloc(sizeof(list_node_t));
        (*list)->data = data;
        (*list)->prev = NULL;
        (*list)->next = NULL;
        return *list;
    }

    // create new node and prepend it to the list
    first_node = *list;
    new_node = (list_node_t *) malloc(sizeof(list_node_t));
    first_node->prev = new_node;
    *list = new_node;

    // set members in new node and return it
    new_node->data = data;
    new_node->prev = NULL;
    new_node->next = first_node;

    return new_node;
}

/** list_append
 *    Allocates a new list node, attaches it to the end of list and sets the
 *    node data pointer to data.
 *    Returns - the new list node.
 */
list_node_t *list_append(list_t *list, void *data)
{
    list_node_t *new_node,
                *last_node;

    if(list_empty(*list))
    {
        (*list) = (list_t) malloc(sizeof(list_node_t));
        (*list)->data = data;
        (*list)->prev = NULL;
        (*list)->next = NULL;
        return *list;
    }

    // find end of list
    last_node = *list;
    while(last_node->next != NULL)
        last_node = last_node->next;

    // create new node and return it
    last_node->next = new_node = (list_node_t *) malloc(sizeof(list_node_t));
    new_node->data = data;
    new_node->prev = last_node;
    new_node->next = NULL;

    return new_node;
}

/** list_node_delete
 *    Deallocates and removes given node from the given list. It is up to the
 *    user to free any memory allocated for the node data before calling this
 *    function. Also, it is assumed that node is an element of list.
 */
void list_node_delete(list_t *list, list_node_t *node)
{
    if(node == NULL || *list == NULL) return;

    if(node->prev != NULL)
        node->prev->next = node->next;
    else
        *list = node->next; // node is first node, update list pointer

    if(node->next != NULL)
        node->next->prev = node->prev;

    free(node);
}

/** list_delete
 *    Deallocates and removes all nodes from the given list. It is up to the
 *    user to free any memory allocated for all node data before calling this
 *    function.
 */
void list_delete(list_t *list)
{
    list_node_t *prev = NULL,
                *curr = NULL;

    // delete all list nodes in the list
    list_foreach(*list, curr)
    {
        if(prev != NULL)
            free(prev);

        prev = curr;
    }
    
    // the last node wasn't deleted, do it now
    if (prev != NULL)
        free(prev);

    *list = NULL;
}

/** list_find_node
 *    Searches the given list for a node whose data pointer matches the given data pointer.
 *    If found, returns a pointer to the node, else, returns NULL.
 */
list_node_t *list_find_node(list_t list, void *data)
{
    list_node_t *node = NULL;

    list_foreach(list, node)
        if(node->data == data)
            break;

    return node;
}

void countrycodestring(char countrycode, char *string)
{
    switch (countrycode)
    {
    case 0:    /* Demo */
        strcpy(string, "Demo");
        break;

    case '7':  /* Beta */
        strcpy(string, "Beta");
        break;

    case 0x41: /* Japan / USA */
        strcpy(string, "USA/Japan");
        break;

    case 0x44: /* Germany */
        strcpy(string, "Germany");
        break;

    case 0x45: /* USA */
        strcpy(string, "USA");
        break;

    case 0x46: /* France */
        strcpy(string, "France");
        break;

    case 'I':  /* Italy */
        strcpy(string, "Italy");
        break;

    case 0x4A: /* Japan */
        strcpy(string, "Japan");
        break;

    case 'S':  /* Spain */
        strcpy(string, "Spain");
        break;

    case 0x55: case 0x59:  /* Australia */
        sprintf(string, "Australia (0x%02X)", countrycode);
        break;

    case 0x50: case 0x58: case 0x20:
    case 0x21: case 0x38: case 0x70:
        sprintf(string, "Europe (0x%02X)", countrycode);
        break;

    default:
        sprintf(string, "Unknown (0x%02X)", countrycode);
        break;
    }
}

void imagestring(unsigned char imagetype, char *string)
{
    switch (imagetype)
    {
    case Z64IMAGE:
        strcpy(string, ".z64 (native)");
        break;
    case V64IMAGE:
        strcpy(string, ".v64 (byteswapped)");
        break;
    case N64IMAGE:
        strcpy(string, ".n64 (wordswapped)");
        break;
    default:
        string[0] = '\0';
    }
}

char* dirfrompath(const char* string)
{
    int stringlength, counter;
    char* buffer;

    stringlength = strlen(string);

    for(counter = stringlength; counter > 0; --counter)
    {
        if (strchr(OSAL_DIR_SEPARATORS, string[counter-1]))
            break;
    }

    buffer = (char*)malloc((counter+1)*sizeof(char));
    snprintf(buffer, counter+1, "%s", string);
    buffer[counter] = '\0';

    return buffer;
}
