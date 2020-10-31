#include "bpt.h"
#include "file.h"
#include "buffer.h"
#include "type.h"

// MAIN

//header_page_t header;

int main( int argc, char ** argv ) {

    pagenum_t root;
    uint64_t input, range2;
    char instruction, value[1024];

    int tid = 1;
    int table1 = open_table("test1.db");
    int table2 = open_table("test2.db");

    header_page_t header;
    init_db(100);
    printf("> ");
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
        case 'd':
            scanf("%lu", &input);
            db_delete(1, input);
            break;
        case 'i':
            scanf("%lu", &input);
            scanf("%s", value);
            db_insert(tid, input, value);
            break;
        case 'f':
            scanf("%lu", &input);
            char ret_val[120];
            int check = db_find(tid, input, ret_val);
            if (check == -1) printf("No matching key in table\n");
            else if (check == 0) printf("Key : %lu, Value : %s\n", input, ret_val);
            break;
        case 'p':
            scanf("%lu", &input);
            //find_and_print(root, input, instruction == 'p');
            break;
        case 'r':
            scanf("%lu %lu", &input, &range2);
            if (input > range2) {
                int tmp = range2;
                range2 = input;
                input = tmp;
            }
            //find_and_print_range(root, input, range2, instruction == 'p');
            break;
        case 'l':
            //print_leaves(root);
            break;
        case 'q':
            while (getchar() != (int)'\n');
            return EXIT_SUCCESS;
            break;
        case 't':
            print_tree(tid);
            //print_tree(root);
            break;
        case 'v':
            //verbose_output = !verbose_output;
            break;
        case 'x':
            if (root)
                //root = destroy_tree(root);
            //print_tree(root);
            break;
        default:
            //usage_2();
            break;
        }
        while (getchar() != (int)'\n');
        printf("> ");
    }
    printf("\n");

    return EXIT_SUCCESS;
}
