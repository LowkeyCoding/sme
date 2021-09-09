#include "sme.h"

int main() {
    int flag = 1;
    char buffer[1024];
    char var_name[32];
    char var_value[32];
    SMEList* vars = new_SMEList();
    while (flag) {
        printf("sme> ");
        scanf("%[^\n]\0", &buffer);
        while (getchar() != '\n');

        if (!strcmp(buffer, "quit")) {
            flag = 0;
            printf("\nQuitting\n");
        }
        else if (buffer[0] == ':') {
            int i = 1;
            int j = 0;
            while (buffer[i] != '=') {
                var_name[j++] = buffer[i++];
            }
            i++;
            var_name[j] = '\0';
            j = 0;
            while (buffer[i]) {
                var_value[j++] = buffer[i++];
            }
            var_value[j] = '\0';
            SMEVar* var = new_SMEVar(var_name, strtod(var_value, NULL));
            append_SMEItem(vars, var);
        }
        else {
            SMETokenizer* tokenizer = sme_tokenize(buffer, vars);
            SMENode* root = sme_parse(tokenizer);
            printf("\nResult: %lf\n", sme_eval(root));
            tokenizer->variables = NULL;
            free_SMETokenizer(tokenizer);
            free_SMENode(root);
        }
    }
    return 0;
}