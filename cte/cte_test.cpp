#include <stdio.h>

int main(int argc, char **argv)
{
    int pre = 100;
    
    BEGIN_METAPROGRAM;
    int num_vars = 10;
    for (int i = 0; i < num_vars; ++i)
    {
        printf("int var%d = %d;\n", i, i);
    }
    
    for (int i = 0; i < num_vars; ++i)
    {
        printf("printf(\"var%d = %%d\\n\", var%d);\n", i, i);
    }
    END_METAPROGRAM;
    
    int post = 2;
    
    return 0;
}

