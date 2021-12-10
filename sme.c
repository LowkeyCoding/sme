#include "sme.h"
#include "./libs/CuTest.h"

SMETokenizer* tokenizer;
SMENode* root;
SMEList *vars;

double values[] = {2,4,2.2,5.4,22,2,33.4,2.4,2.4};

void test_tokenizer(CuTest* tc){
    vars = new_SMEList();
    tokenizer = sme_tokenize("+(2 * (4 / (2.2 + -5.4) - 22) * 2 + ceil(floor(33.4 + 2.4) * 2.4))", vars);

    int types[] = {
            SMEAdd, SMELP, SMENum, SMEMul, SMELP, SMENum, SMEDiv,
            SMELP, SMENum, SMEAdd, SMESub, SMENum, SMERP, SMESub,
            SMENum, SMERP, SMEMul, SMENum, SMEAdd, SMECeil, SMELP,
            SMEFloor, SMELP, SMENum, SMEAdd, SMENum, SMERP, SMEMul,
            SMENum, SMERP, SMERP
    };

    int j = 0;
    for(int i=0; i < tokenizer->list->count; i++){
        SMEToken* token = tokenizer->list->items[i];
        CuAssertIntEquals(tc, types[i], token->type);
        if(token->type == SMENum){
            CuAssertDblEquals(tc, values[j], token->value, 0.001);
            j++;
        }
    }
}

int node_types[] = {
        SMENum, SMENum, SMENum, SMENum, SMENeg, SMEAdd, SMEDiv,
        SMENum, SMESub, SMENum, SMEMul, SMEMul, SMENum, SMENum,
        SMEAdd, SMEFloor, SMENum, SMEMul, SMECeil, SMEAdd, SMEPos
};

int node_index = 0;
int value_index = 0;

void test_parser_recursion(CuTest* tc, SMENode* node){
    if(node->left)
        test_parser_recursion(tc, node->left);
    if(node->right)
        test_parser_recursion(tc, node->right);
    CuAssertIntEquals(tc, node_types[node_index], node->type);
    node_index++;

    if(node->type == SMENum){
        CuAssertDblEquals(tc, values[value_index], node->value, 0.001);
        value_index++;
    }
}

void test_parser(CuTest* tc){
    root = sme_parse(tokenizer);
    test_parser_recursion(tc, root);
    node_index = 0;
    value_index = 0;
}

void test_evaluator(CuTest* tc){
    double result = sme_eval(root);
    CuAssertDblEquals(tc, 9, result, 0.001);
    free_SMETokenizer(tokenizer);
    free_SMENode(root);
}

void test_variables(CuTest* tc){
    vars = new_SMEList();
    append_SMEItem(vars, new_SMEVar("a", 3.4));
    append_SMEItem(vars, new_SMEVar("b", 5.6));
    append_SMEItem(vars, new_SMEVar("x", -9.23));
    append_SMEItem(vars, new_SMEVar("y", 2));
    tokenizer = sme_tokenize("a + b * x / y", vars);
    int ttypes[] = { SMENum, SMEAdd, SMENum, SMEMul, SMENum, SMEDiv, SMENum };
    double tvalues[] = { 3.4, 5.6, -9.23, 2};
    int j = 0;
    for(int i=0; i < tokenizer->list->count; i++){
        SMEToken* token = tokenizer->list->items[i];
        CuAssertIntEquals(tc, ttypes[i], token->type);
        if(token->type == SMENum){
            CuAssertDblEquals(tc, tvalues[j], token->value, 0.001);
            j++;
        }
    }
    free_SMETokenizer(tokenizer);
}

/* Add all the tests to the test suite. */
CuSuite* test_suite() {
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_tokenizer);
    SUITE_ADD_TEST(suite, test_parser);
    SUITE_ADD_TEST(suite, test_evaluator);
    SUITE_ADD_TEST(suite, test_variables);
    return suite;
}

/* Runs all the tests and prints the result. */
void all_tests() {
    CuString *output = CuStringNew();
    CuSuite *suite = CuSuiteNew();

    CuSuiteAddSuite(suite, test_suite());

    CuSuiteRun(suite);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
}


int main(void) {
    all_tests();
    return 0;
}