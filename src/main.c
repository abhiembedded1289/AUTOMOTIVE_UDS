/* Interactive command-line tester: parses hex UDS payloads and injects them
 * into the simulated communication stack. */
#include "dcm.h"
#include "project_types.h"
#include "sim_can.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

static void PrintHelp(void)
{
    /* List available shell commands and a few valid UDS request examples. */
    puts("Commands:");
    puts("  send <byte> [byte ...]  Send 1 to 7 UDS bytes (hexadecimal)");
    puts("  help                    Show this help");
    puts("  quit                    Exit the simulator");
    puts("");
    puts("Examples:");
    puts("  send 10 03");
    puts("  send 27 01");
    puts("  send 27 02 B7 91");
    puts("  send 22 F1 A0");
}

/* Parse one hex byte without accepting partial values such as "2G". */
static int ParseHexByte(const char *text, uint8_t *value)
{
    char *end = NULL;
    unsigned long parsed;

    if (text == NULL || value == NULL || text[0] == '\0' || text[0] == '-')
    {
        return 0;
    }

    parsed = strtoul(text, &end, 16);
    if (end == text || *end != '\0' || parsed > 0xFFUL)
    {
        return 0;
    }

    *value = (uint8_t)parsed;
    return 1;
}

/* Return the next whitespace-delimited token, modifying the input line in place. */
static char *NextToken(char **cursor)
{
    char *token;

    while (**cursor != '\0' && isspace((unsigned char)**cursor))
    {
        (*cursor)++;
    }

    if (**cursor == '\0')
    {
        return NULL;
    }

    token = *cursor;
    while (**cursor != '\0' && !isspace((unsigned char)**cursor))
    {
        (*cursor)++;
    }
    if (**cursor != '\0')
    {
        **cursor = '\0';
        (*cursor)++;
    }
    return token;
}

static int EqualsIgnoreCase(const char *left, const char *right)
{
    /* Compare full command words while treating ASCII letter case as equal. */
    while (*left != '\0' && *right != '\0')
    {
        if (tolower((unsigned char)*left) != tolower((unsigned char)*right))
        {
            return 0;
        }
        left++;
        right++;
    }

    return *left == '\0' && *right == '\0';
}

static void HandleSend(char *cursor)
{
    /* Parse the send command's hex bytes, validate their count, and inject them. */
    uint8_t request[DEMO_MAX_UDS_LENGTH];
    size_t request_length = 0U;
    char *token = NextToken(&cursor);

    while (token != NULL)
    {
        if (request_length >= DEMO_MAX_UDS_LENGTH)
        {
            puts("Error: single-frame requests are limited to 7 UDS bytes.");
            return;
        }
        if (!ParseHexByte(token, &request[request_length]))
        {
            printf("Error: '%s' is not a hexadecimal byte from 00 to FF.\n",
                   token);
            return;
        }

        request_length++;
        token = NextToken(&cursor);
    }

    if (request_length == 0U)
    {
        puts("Error: provide at least one UDS byte. Example: send 10 03");
        return;
    }

    if (SimCan_SendUdsRequest(request, request_length) != E_OK)
    {
        puts("Error: virtual CAN transport could not send the request.");
    }
}

int main(void)
{
    /* Initialize DCM state, then keep accepting tester commands until exit/EOF. */
    char line[256];

    Dcm_Init();
    puts("AUTOSAR-style UDS virtual CAN simulator");
    puts("Type 'help' for examples. No CAN hardware is used.");

    while (1)
    {
        char *cursor = line;
        char *command;

        printf("\nuds> ");
        if (fgets(line, sizeof(line), stdin) == NULL)
        {
            break;
        }

        command = NextToken(&cursor);
        if (command == NULL)
        {
            continue;
        }

        /* Commands are case-insensitive to make the small shell forgiving. */
        if (EqualsIgnoreCase(command, "help"))
        {
            PrintHelp();
        }
        else if (EqualsIgnoreCase(command, "quit") ||
                 EqualsIgnoreCase(command, "exit"))
        {
            break;
        }
        else if (EqualsIgnoreCase(command, "send"))
        {
            HandleSend(cursor);
        }
        else
        {
            printf("Unknown command: %s (type 'help')\n", command);
        }
    }

    puts("Simulator stopped.");
    return EXIT_SUCCESS;
}
