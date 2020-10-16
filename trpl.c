#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int **statetranstab; //двумерный массив для хранения переходов между состояниями. statetranstab[i][j] это состояние, достигаемое, когда состоянию i соответствует символ j
int **partitiontranstab; // то же самое за исключением индексов строк - они представляют номера разбиений, а не номера состояний
int start; //начальное состояние, использующееся как корень для DFS для устранения недостижимых состояний
long int reachable; // битовый набор для представления состояний, которые достижимы
long int all; // битовый набор для представления всех состояний
long int final; // битовый набор для представления конечных состояний
long int nonfinal; // битовый набор для представления неконечных состояний
long int *bittab; // массив разбиений, каждое из которых представляет собой битовый набор состояний

void dfs(int k) //будем использовать для определения достижимых состояний
{
    //побитовое или с присваиванием
    reachable = reachable | (1 << k); //добавление бита на k-место в битовый набор
	
    // пытаемся изучить все пути
    for (int i=0; i<26; i++)
    if((statetranstab[k][i] != -1) && ((reachable & (1 << statetranstab[k][i])) == 0))
    {
        dfs(statetranstab[k][i]); //поиск в глубину
    }
}

int main()
{
    // инициализируем массивы - устанавливаем переход [i][j]=-1, чтобы показать, что состояние/разбиение i не переходит при заданном символе j
    // 50 - максимальное число состояний, параметр можно изменить; 26 - латинский алфавит, также можно изменить, чаще всего используются только a и b
    statetranstab = (int**)malloc(50*sizeof(int*));
    for (int i = 0; i < 50; i++)
    {
        statetranstab[i] = (int*) malloc(26*sizeof(int));
        for (int j = 0; j < 26; j++)
        {
            statetranstab[i][j] = -1;
        }
    }

    partitiontranstab = (int**)malloc(50*sizeof(int*));
    for (int i = 0; i < 50; i++)
    {
        partitiontranstab[i] = (int*) malloc(26*sizeof(int));
        for (int j = 0; j < 26; j++)
        {
            partitiontranstab[i][j] = -1;
        }
    }

    // начинаем без состояний
    final = 0;
    all = 0;

    // считываем начальное состояние
    char buff[200];
    fgets(buff, sizeof(buff), stdin); //ввод с клавиатуры и помещение в строку
    char *p = strtok(buff, " "); //поиск символов разделителей, возвращает указатель на последнюю лексему
    start = atoi(p); //преобразуем строку в int

    // считываем конечные состояния
    fgets(buff, sizeof(buff), stdin);
    p = strtok(buff, " ");
    while (p != NULL) //пока есть лексеммы
    {
        int state = atoi(p);
        final = final | 1 << (state);
        p = strtok(NULL, " ");
    }

    // считываем переходы
    int from;
    char symbol;
    int to;
    while (fscanf(stdin, "%d %c %d", &from, &symbol, &to) != EOF) 
    {
        statetranstab[from][symbol-'a'] = to; // добавляем переход
	
        // добавляем from и to состояния в битовый набор all
        all = all | (1 << from); 
        all = all | (1 << to);
    }

    // инициализируем битовый набор reachable нулем и запускаем dfs для определения достижимых состояний
    reachable = 0;
    dfs(start);

    // фильтруем недостижимые состояния
    all = all & reachable; //побитовое и с присваиванием - все, что равно нулю в reachable, становится нулем в all, остальное сохраняется
    final = final & reachable;

    // инициализируем массив разбиений, чтобы включить пустые битовые наборы
    bittab =  (long int*) malloc(50*sizeof(long int));
    for (int i = 0; i < 50 ; i++)
    {
        bittab[i] = 0; // нет разбиений
    }

    // для работы bittab должен включать два разбиения: конечные и неконечные состояния
    nonfinal = all & ~final; //побитовое не
    bittab[0] = final;
    bittab[1] = nonfinal;

    int nextpartitionindex = 2; // запоминаем, сколько разбиений уже было добавлено

    // будет не более 50 разбиений, на каждом шаге цикла мы работаем с разбиением и добавляем не более одного разбиения
    for (int i = 0; i < 50; i++)
    {

        // биотвый набор для нового разбиения, который будет включать все состояния, отличные от того, который соответствует самому левому биту в P[i]
        long int newpartition = 0;

        // разбиение готово
        if (bittab[i] == 0)
        {
            break;
        }

        // пытаемся найти самый левый бит в битовом наборе (цикл будет работать только один раз, когда этот бит будет найден)
        for (int j = 49; j >=  0; j--) 
        {

            // потенциальный левый бит: если он найден, он останется в битовом наборе
            long int statics = (long int) 1 << j;

            // проверяем, находится ли это состояние в текущем битовом наборе
            if ((bittab[i] & (statics)) != 0)
            {

                // крайнее состояние, соответствующее саомму левому биту, будет связано с этим разбиением, поэтому мы должны скопировать переходы для этого состояния в переходы для соответствующего разбиения
                partitiontranstab[i] = statetranstab[j];

                // проверяем на наличие состояний, которые следует удалить из этого разбиения: все состояния будут битами справа от бита statics
                for (int k = j - 1; k >= 0; k -- )
                {

                    // потенциальное состояние для удаления
                    long int suspect = (long int) 1 << k;

                    // проверяем, находится ли состояние в текущем битовом наборе
                    if ((bittab[i] & (suspect)) != 0)
                    {

                        // проходим по всему алфавиту и проверяем, могут ли statics и suspect переходить в разные разбиения
                        for (int l  = 0; l < 26; l++)
                        {

                            int nextstatics = -1; // следующее разбиение для statics
                            int nextsuspect = -1; // следующее разбиение для suspect

                            for (int m = 0; m < nextpartitionindex; m++)
                            {
                                if ((bittab[m] & (1 << statetranstab[j][l])) != 0)
                                {
                                    nextstatics = m; //находим nextstatics
                                }
                                if ((bittab[m] & (1 << statetranstab[k][l])) != 0)
                                {
                                    nextsuspect = m; // находим nextsuspect
                                }
                            }

                            // если разбиения отличаются, удаляем состояние suspect и добавляем в новое разбиение, затем пауза, поскольку закончили работу с этим разбиением
                            if (statetranstab[j][l] != statetranstab[k][l] && (nextstatics != nextsuspect))
                            {
                                bittab[i] = bittab[i] & ~(1 << k);//удаляем бит с k-места
                                newpartition = newpartition | (1 << k);
                                break;
                            } 
                        }	
                    }
                }
                break;
            }
        }

        // новое разбиение существует: добавляем его в bittab и увеличиваем nextpartitionindex
        if (newpartition != 0)
        {
            bittab[nextpartitionindex] = newpartition;
            nextpartitionindex++;
        }
    }

    // находим и печатаем начальное разбиение
    int startpartition = 0;
    for (int i = 0; i < nextpartitionindex; i ++)
    {
        if ((bittab[i] & (1 << start)) != 0 )
        {
            startpartition = i;
            break;
        }
    }

    printf("%d \n", startpartition);

    // находим и печатаем финальные разбиения
    for (int i = 0; i < nextpartitionindex; i++)
    {
        if ((bittab[i] & final) != 0)
        {
            printf("%d ", i);
        }
    }
    printf("\n");

    // находим и печатаем все переходы
    for (int i = 0; i < nextpartitionindex; i++)
    {
        for (int j = 0; j < 26; j++) 
        {
            if (partitiontranstab[i][j] != -1)
            {
                for (int k = 0; k < nextpartitionindex; k++)
                {
                    if ((bittab[k] & (1 << partitiontranstab[i][j])) != 0)
                    {
                        printf("%d %c %d\n", i, j + 'a', k);
                    }
                }
            }
        }
    }
    return 0;
}
