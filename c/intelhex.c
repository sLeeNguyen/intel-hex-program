#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define false 0
#define true 1

typedef uint8_t bool;

const uint8_t MAX_LINE_DISPLAYED = 25;
const char RECORD_START_CODE = ':';
const uint8_t MAX_BYTE_COUNT = 255;
const uint16_t HEX_LINE_MAX_LENGTH = 521;
enum RECORD_TYPES
{
  DATA = 0,
  HEX_EOF = 1,
  EXTENDED_SEGMENT_ADDRESS = 2,
  START_SEGMENT_ADDRESS = 3,
  EXTENDED_LINEAR_ADDRESS = 4,
  START_LINEAR_ADDRESS = 5
};

/** A struct represents an Intel Hex Record (IHR)
 * Intel Hex Record format:
 *    :10246200464C5549442050524F46494C4500464C33
 *    |||||||||||                              CC->Checksum
 *    |||||||||DD->Data
 *    |||||||TT->Record Type
 *    |||AAAA->Address
 *    |LL->Record Length
 *    :->Colon
 * 
 * See https://www.keil.com/support/docs/1584/ for more detail.
 */
struct Record
{
  uint8_t len;
  uint16_t addr;
  uint8_t type;
  uint8_t data[255];
  uint8_t check;
};
typedef struct Record Record;

struct IntelHexArray
{
  struct Record *records;
  uint16_t size;
  uint16_t capacity;
};
typedef struct IntelHexArray IntelHexArray;

/** A function to start the program.
 */
void start(const char *filepath);

/** Function to handle view file request.
 */
void start_reading_file(const IntelHexArray *obj);

/** Function to handle save file request.
 */
void start_save_to_file(const IntelHexArray *obj);

/** Function to check and open file.
 * 
 * @param filename A pointer to const char*.
 * @param mode A pointer to cont char*. This pram specifies 
 *             the working mode with the file.
 * @return A pointer to FILE*.
 */
FILE *open_file(const char *filename, const char *mode);

/** Function to read line from hex file and
 * converts it to a Record.
 * 
 * @param file A pointer to a FILE*.
 * @return An IntelHexArray contains all data Records 
 *         (record type is 00).
*/
IntelHexArray convert_hex_file_to_array(FILE *file);

/** Function to convert hex digit to decimal.
 * 
 * @param ch A variable of type char represents a hex digit.
 * @return A decimal of hex digit in range [0, 15]. if `ch` is
 *         not a valid hex digit, 16 will be returned.
 */
uint8_t hex2dec(const char ch);

/** Function to parse a hex line to Record.
 * 
 * @param line A pointer to uint8_t*.
 * @param record A pointer to Record*. This param will hold 
*                data parsed from `line`.
 * @return 0 if `line` is invalid hex digits, otherwise 1 
 *         will be returned.
*/
bool parse_intel_hex_line(uint8_t *const line, Record *record);

/** Function to get one byte at `start` position in a hex 
 * string `line`.
 * 
 * @param line A pointer to a hex string.
 * @param start The start position of hex digit will be 
 *              converted to byte.
 * @param byte A reference to a variable of uint8_t to hold
 *             the byte value.
 * @param readed_bytes The number of bytes has been read in 
 *                     hex string `line`.
 * @return A boolean value representing the execution status 
 *         of success or failure.
 */
bool get_byte(uint8_t *const line, const uint16_t start, uint8_t *byte, uint16_t *readed_bytes);

/** Function to check a Record is a data record or not.
 * 
 * @return true if Record is a data record, otherwise returns false.
 */
bool is_data_record(Record record);

/** Function to convert a string of hex digits to array of bytes.
 * 
 * @return true if convert success, otherwise returns false.
 */
bool convert_hex_line_to_bytes(uint8_t *const line, uint8_t *array, uint16_t *readed_bytes);

/** Function to validate a Record.
 * 
 * @return true if Record is valid, false otherwise.
 */
bool validate_record(const Record record);

/** Function to display a valid Record to console.
 * 
 * @param record A struct of Record will be displayed.
 */
void display_record(const Record record);

void display_hex_data(IntelHexArray const *obj, uint16_t *from);
int wait_user_choose_menu();
void display_menu();

/** Function to flush buffer
 */
void flush_stdin()
{
  int c;
  while ((c = getchar()) != '\n' && c != EOF)
  {
  };
}

/** Function to clear console.
 */
void clear_console()
{
  #if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        system("clear");
  #endif

  #if defined(_WIN32) || defined(_WIN64)
      system("cls");
  #endif
}

/* Implement intel hex functions */

IntelHexArray convert_hex_file_to_array(FILE *file)
{
  IntelHexArray obj;

  // length of records
  uint16_t len = 10;
  // next record index
  uint16_t next = 0;
  // using dynamic allocate
  Record *records = (Record *)malloc(len * sizeof(Record));

  uint8_t line[HEX_LINE_MAX_LENGTH];
  while (fgets(line, HEX_LINE_MAX_LENGTH, file))
  {
    Record record;
    int success = parse_intel_hex_line(line, &record);
    if (success)
    {
      records[next++] = record;
      // printf("success read record at line %d\n", next);
    }
    else
    {
      printf("ERROR line %d: Record is malformed.", next + 1);
      exit(1);
    }
    if (next >= len)
    {
      // resize records
      len += 10;
      records = realloc(records, len * sizeof(Record));
    }
  }

  obj.records = records;
  obj.size = next;
  obj.capacity = len;
  return obj;
}

bool parse_intel_hex_line(uint8_t *const line, Record *record)
{
  const uint16_t max_bytes = HEX_LINE_MAX_LENGTH / 2;
  uint16_t size = 0;
  uint8_t *bytes_array = (uint8_t *)malloc(max_bytes * sizeof(uint8_t));
  if (bytes_array == NULL)
  {
    printf("ERROR: Memory not allocated.\n");
    exit(1);
  }
  if (!convert_hex_line_to_bytes(line, bytes_array, &size))
  {
    free(bytes_array);
    bytes_array = NULL;
    return false;
  }
  record->len = bytes_array[0];
  record->addr = (bytes_array[1] << 8) | bytes_array[2];
  record->type = bytes_array[3];
  record->check = bytes_array[size - 1];

  int j = 0;
  for (int i = 4; i < size - 1; ++i)
  {
    record->data[j++] = bytes_array[i];
  }

  // printf("=============================\n");
  // printf("byte count: %d\n", record->len);
  // printf("address: %05x\n", record->addr);
  // printf("record type: %d\n", record->type);
  // printf("check sum: %d\n", record->check);
  // printf("=============================\n");
  // validate_record(*record);

  return 1;
}

bool convert_hex_line_to_bytes(uint8_t *const line, uint8_t *array, uint16_t *readed_bytes)
{
  if (line[0] != (uint8_t)RECORD_START_CODE)
  {
    // puts("invalid hex line");
    return false;
  }
  int i = 1; // for hex line
  int j = 0; // for byte array
  while (line[i] != '\0' && line[i] != '\n')
  {
    bool check = get_byte(line, (uint16_t)i, array + j, readed_bytes);
    // printf(" %d\n", array[j]);
    if (!check) // invalid hex line
    {
      return false;
    }

    i += 2;
    j++;
  }

  return true;
}

bool get_byte(uint8_t *const line, const uint16_t start, uint8_t *byte, uint16_t *readed_bytes)
{
  // each byte corresponding to two hex digits
  // byte ~ <hex1><hex2> --> byte = hex1 * 16 + hex2
  uint8_t hex1 = hex2dec(line[start]);
  uint8_t hex2 = hex2dec(line[start + 1]);
  // printf("%01x %01x", hex1, hex2);
  if (hex1 == 16 || hex2 == 16)
  {
    return false;
  }

  *byte = (hex1 << 4) + hex2;
  // printf(" %d", *byte);
  (*readed_bytes)++;

  return true;
}

uint8_t hex2dec(const char ch)
{
  if (ch >= '0' && ch <= '9')
  {
    return (uint8_t)(ch - '0');
  }
  if (ch >= 'A' && ch <= 'Z')
  {
    return (uint8_t)(ch - 'A' + 10);
  }
  if (ch >= 'a' && ch <= 'z')
  {
    return (uint8_t)(ch - 'a' + 10);
  }
  return 16; // ch is not a valid hex digit.
}

bool is_data_record(Record record)
{
  return record.type == DATA;
}

bool validate_record(const Record record)
{
  uint16_t aaaa = record.addr;
  uint8_t high_addr = (uint8_t)(aaaa >> 8);
  uint8_t low_addr = (uint8_t)(aaaa & 0x00ff);
  uint8_t sum = (uint8_t)record.len + high_addr + low_addr + record.type;
  for (int i = 0; i < record.len; ++i)
  {
    sum = (uint8_t)sum + record.data[i];
  }
  uint8_t two_complement = ~sum + 0x01;
  // printf("two_complement: %d\n", two_complement);
  // printf("checksum: %d\n", record.check);
  return two_complement == record.check;
}

FILE *open_file(const char *filename, const char *mode)
{
  FILE *fptr;
  fptr = fopen(filename, mode);
  if (fptr == NULL)
  {
    printf("ERROR: %s could not be opened.\n", filename);
    exit(1);
  }

  return fptr;
}

/* ui functions */

void display_menu()
{
  printf("===================== Intel Hex Program Menu =====================\n\n");
  printf("\t\t1. View file in console\n");
  printf("\t\t2. Save file\n");
  printf("\t\t3. Quit\n");
  printf("\nYour choice: ");
}

void display_record(const Record record)
{
  // address cell
  printf("%06x\t", record.addr);

  // hex data cell
  for (int i = 0; i < record.len; ++i)
  {
    printf("%02x ", record.data[i]);
  }
  printf("\t");

  // ascii data cell
  for (int i = 0; i < record.len; ++i)
  {
    printf("%c", record.data[i]);
  }
  printf("\n");
}

void display_hex_data(IntelHexArray const *obj, uint16_t *from)
{
  uint8_t records_diplayed = 0;
  for (; *from < obj->size && records_diplayed < MAX_LINE_DISPLAYED; ++(*from))
  {
    Record r = obj->records[*from];
    // if (validate_record(r) && is_data_record(r))
    if (is_data_record(r))
    {
      display_record(r);
      records_diplayed++;
    }
  }
}

int wait_user_choose_menu()
{
  int choosen;
  scanf("%d", &choosen);
  flush_stdin();
  return choosen;
}

void start_reading_file(const IntelHexArray *obj)
{
  clear_console();
  uint16_t current_line = 0;
  char ans;

  do
  {
    clear_console();
    display_hex_data(obj, &current_line);
    if (current_line < obj->size)
    {
      do
      {
        printf("\nPress `y` to see more 25 lines or press `n` to stop program. ");
        scanf("%c", &ans);
        flush_stdin();
      } while (ans != 'y' && ans != 'n');
    }
    else
    {
      printf("\n=========== End of file ===========");
      getchar(); // wait user press key
      break;
    }
    printf("\n");
  } while (ans == 'y');
}

void start_save_to_file(const IntelHexArray *obj)
{
  clear_console();
  char filename[255];

  printf("Enter file name (must has .txt extension): ");
  scanf("%s", filename);

  int len = strlen(filename);

  // check filename has .txt extension
  bool is_txt_file = !(strcmp(filename + (len - 4), ".txt"));
  if (!is_txt_file)
  {
    printf("ERROR: %s is not a valid file.", filename);
    exit(1);
  }

  // push data to file.
  FILE *fptr = open_file(filename, "w");

  for (int i = 0; i < obj->size; ++i)
  {
    Record r = obj->records[i];
    if (is_data_record(r))
    {
      // push address
      fprintf(fptr, "%06x\t", r.addr);

      // push data in hex format
      for (int j = 0; j < r.len; ++j)
      {
        fprintf(fptr, "%02x ", r.data[j]);
      }

      fprintf(fptr, "\t");

      // push data in dec format
      for (int j = 0; j < r.len; ++j)
      {
        fprintf(fptr, "%c ", r.data[j]);
      }

      fprintf(fptr, "\n");
    }
  }

  fclose(fptr);
  flush_stdin();
  printf("\nSaved successfully.\n");
  printf("\nPress any key to go to menu. ");
  getchar(); // wait user press key
}

void start(const char *filepath)
{
  FILE *hexFile = open_file(filepath, "r");

  printf("Loading file ...\n");
  IntelHexArray obj = convert_hex_file_to_array(hexFile);
  fclose(hexFile);

  int choice;
  do
  {
    clear_console();
    display_menu();
    choice = wait_user_choose_menu();
    switch (choice)
    {
    case 1:
      start_reading_file(&obj);
      break;
    case 2:
      start_save_to_file(&obj);
      break;
    default:
      break;
    }
  } while (choice != 3);
}

/* main program */

int main(int argc, char *argv[])
{
  char filepath[255];
  if (argc > 1)
  {
    strcpy(filepath, argv[1]);
  }
  else
  {
    printf("ERROR: Missing one parameter contains the path to hex file.");
    exit(1);
  }

  // start program
  start(filepath);

  return 0;
}