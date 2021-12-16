#include <stdio.h>
#include <stdlib.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif
#include <time.h> //clock(), CLOCKS_PER_SEC e clock_t
#define ARRAY_LENGTH 25

int main(int argc, char** argv) {
    /* Variáveis para armazenamento de referências a
    objetos OpenCL */
    cl_platform_id platformId;
    cl_device_id deviceId;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_int err;
    cl_mem bufValorA;
    cl_mem bufValorB;
    cl_mem bufValorC;
    cl_mem bufContador;
    size_t globalSize[1] = { ARRAY_LENGTH };

    /* Variáveis diversas da aplicação */
    int* hostValorA;
    int* hostValorB;
    int* hostValorC;
    int* hostContador;
    int i;

    /* Variável para a criação do arquivo */
    FILE * fPtr;

    //variável para armazenar tempo
    clock_t time_host, time_kernel;

    /* Código-fonte do kernel */
    const char* source = 
    "__kernel void Triplas(__global int* a, __global int* b, __global int* c, __global int* contador) \
        { \
            int x,j,k; \
            int quantidade = 100; \
            int id = get_global_id(0); \
            for (x = 1; x < quantidade; ++x){ \
                for (j = x; j < quantidade; ++j){ \
                    for (k = j; k <= quantidade; ++k){ \
                        if( (x*x)+(j*j)==(k*k) ){\
                            contador[0]+=1; \
                            a[contador[0]]=x;\
                            b[contador[0]]=j;\
                            c[contador[0]]=k;\
                        } \
                    } \
                } \
            } \
        }";

            /* Alocação e inicialização dos arrays no hospedeiro */
        hostValorA = (int*) malloc(ARRAY_LENGTH * sizeof(int));
        hostValorB = (int*) malloc(ARRAY_LENGTH * sizeof(int));
        hostValorC = (int*) malloc(ARRAY_LENGTH * sizeof(int));
        hostContador = (int*) malloc(ARRAY_LENGTH * sizeof(int));


    /* Obtenção de identificadores de plataforma e dispositivo. Será solicitada uma GPU. */
    clGetPlatformIDs(1, &platformId, NULL);
    clGetDeviceIDs(platformId, CL_DEVICE_TYPE_GPU, 1, &deviceId, NULL);

    /* Criação do contexto */
    context = clCreateContext(0, 1, &deviceId, NULL, NULL, NULL);

    /* Criação da fila de comandos para o dispositivo encontrado */
    queue = clCreateCommandQueue(context, deviceId, 0, NULL);

    /* Criação do objeto de programa a partir do código-fonte armazenado na string source */
    program = clCreateProgramWithSource(context, 1, &source, NULL, NULL);

    /* Compilação do programa para todos os dispositivos do contexto */
    clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

    /* Obtenção de um kernel a partir do programa compilado */
    kernel = clCreateKernel(program, "Triplas", NULL);

    /* Criação dos objetos de memória para comunicação com a memória global do dispositivo encontrado */
    bufValorA = clCreateBuffer(context, CL_MEM_READ_WRITE, ARRAY_LENGTH * sizeof(int), NULL, NULL);
    bufValorB = clCreateBuffer(context, CL_MEM_READ_WRITE, ARRAY_LENGTH * sizeof(int), NULL, NULL);
    bufValorC = clCreateBuffer(context, CL_MEM_READ_WRITE, ARRAY_LENGTH * sizeof(int), NULL, NULL);
    bufContador = clCreateBuffer(context, CL_MEM_READ_WRITE, ARRAY_LENGTH * sizeof(int), NULL, NULL);
    

    /* Transferência dos arrays de entrada para a memória do dispositivo */
    // clEnqueueWriteBuffer(queue, bufValorA, CL_TRUE, 0, ARRAY_LENGTH * sizeof(int), hostValorA, 0, NULL, NULL);
   
    /* Configuração dos argumentos do kernel */
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufValorA);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufValorC);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufValorB);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &bufContador);
    

    /* Envio do kernel para execução no dispositivo */
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, globalSize, NULL, 0, NULL, NULL);

    /* Sincronização (bloqueia hospedeiro até término da execução do kernel */
    clFinish(queue);

    /* Transferência dos resultados da computação para a memória do hospedeiro */
    clEnqueueReadBuffer(queue, bufValorA, CL_TRUE, 0, ARRAY_LENGTH * sizeof(int), hostValorA, 0, NULL, NULL);
    clEnqueueReadBuffer(queue, bufValorB, CL_TRUE, 0, ARRAY_LENGTH * sizeof(int), hostValorB, 0, NULL, NULL);
    clEnqueueReadBuffer(queue, bufValorC, CL_TRUE, 0, ARRAY_LENGTH * sizeof(int), hostValorC, 0, NULL, NULL);
    clEnqueueReadBuffer(queue, bufContador, CL_TRUE, 0, ARRAY_LENGTH * sizeof(int), hostContador, 0, NULL, NULL);

   
    fPtr = fopen("tripla.txt", "w");

    /* caso a criação do arquivo não tenha sido bem sucedida. */
    if(fPtr == NULL)
    {
        printf("Não foi possível criar o arquivo.\n");
        exit(EXIT_FAILURE);
    } else {

        fprintf(fPtr,"\n  ✩｡:*•.─────  ❁  Triplas Pitagóricas ❁  ─────.•*:｡✩  \n \n");

        /* Impressão dos resultados na saída padrão */
        for (i = 1; i <= hostContador[0]; ++i){
            // printf("i = %d c=%d\n",i,hostValorC[i]);
            printf("a = %d b=%d  c=%d\n",hostValorA[i], hostValorB[i], hostValorC[i]);
            fprintf(fPtr,"(a = %d, b= %d, c=%d) \n",hostValorA[i], hostValorB[i], hostValorC[i]);
        }
    }
        printf("contador = %d\n", hostContador[0]);
        fprintf(fPtr,"\n Total de triplas: %d",hostContador[0]);
        fclose(fPtr);

    /* Liberação de recursos e encerramento da aplicação */
    clReleaseMemObject(bufValorA);
    clReleaseMemObject(bufValorB);
    clReleaseMemObject(bufValorC);
    clReleaseMemObject(bufContador);
    

    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    free(hostValorA);
    free(hostValorB);
    free(hostValorC);
    free(hostContador);

    return 0;
}