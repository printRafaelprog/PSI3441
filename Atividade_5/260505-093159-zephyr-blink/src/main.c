#include "ultrasom.h"

int main(void)
{   
    float dist = 0.0;
    while (1)
    {   
        dist = ultrasom_dist(); // Chama a função para obter a distância
        printf("Distancia: %.2f mm\n", dist);
    }
    return 0;
}