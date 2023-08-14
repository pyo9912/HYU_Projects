#include "bpt.h"
#include "file.h"

// MAIN

//header_page_t header;

int main( int argc, char ** argv ) {

    pagenum_t root;
    uint64_t input, range2;
    char instruction, value[1024];

    open_table("sample_10000.db");

    header_page_t header;
    //header_page_t* header = (header_page_t*)malloc(PAGESIZE);
    //file_read_page(0, ((page_t*)&header));

    printf("> ");
    while (scanf("%c", &instruction) != EOF) {
        file_read_page(0,(page_t*)&header);
        root = (&header)->root_page_number;
        switch (instruction) {
        case 'd':
            scanf("%lu", &input);
            db_delete(input);
            //print_tree(root);
            break;
        case 'i':
            scanf("%lu", &input);
            scanf("%s", value);
            db_insert(input, value);
            //print_tree(root);
            break;
        case 'f':
            scanf("%lu", &input);
            char ret_val[120];
            int check = db_find(input, ret_val);
            if (check == 0) printf("Key : %lu, Value : %s\n", input, ret_val);
            else printf("No Exist\n");
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
