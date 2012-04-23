// GA-Power.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <stdio.h>


// TODO: reference additional headers your program requires here
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <assert.h>


const int N = 32;
const int M = 5000;
const int C = 6;

const int UNIT_POWER_PLANT = 0;
const int UNIT_ACCUMULATOR = 1;
const int UNIT_CRYSTAL     = 2;

const int UNIT_CRYSTAL_0      = 0;

const int UNIT_POWER_PLANT_12 = UNIT_CRYSTAL_0 + 1;
const int UNIT_POWER_PLANT_15 = UNIT_POWER_PLANT_12 + 1;
const int UNIT_POWER_PLANT_16 = UNIT_POWER_PLANT_15 + 1;
const int UNIT_POWER_PLANT_17 = UNIT_POWER_PLANT_16 + 1;
const int UNIT_POWER_PLANT_19 = UNIT_POWER_PLANT_17 + 1;

const int UNIT_ACCUMULATOR_12 = UNIT_POWER_PLANT_19 + 1;
const int UNIT_ACCUMULATOR_19 = UNIT_ACCUMULATOR_12 + 1;
const int UNIT_ACCUMULATOR_20 = UNIT_ACCUMULATOR_19 + 1;


struct unit_param {
  int type;
  int constant, acc1, acc2;
};


unit_param params[] = {
//      type         pack   cry   acc
  {UNIT_CRYSTAL    ,    0,    0,    0},
  
  {UNIT_POWER_PLANT, 1210,  605,  726},  // 12
  {UNIT_POWER_PLANT, 2091, 1045, 1254},  // 15
  {UNIT_POWER_PLANT, 2509, 1254, 1505},  // 16
  {UNIT_POWER_PLANT, 3011, 1505, 1806},  // 17
  {UNIT_POWER_PLANT, 4336, 2168, 2601},  // 19

  {UNIT_ACCUMULATOR, 0   ,  484,    0},  // 12
  {UNIT_ACCUMULATOR, 0   , 1734,    0},  // 19
  {UNIT_ACCUMULATOR, 0   , 2082,    0}   // 20
};


struct unit_instance {
  bool operator==(const unit_instance &b) const { return param == b.param && row == b.row && col == b.col; }
  bool operator!=(const unit_instance &b) const { return !(*this == b); }
  unit_param *param;
  int row, col;
};


struct individual {
  bool operator==(const individual &b) const { 
    for (int i = 0; i < N; ++i) {
      if (unit[i] != b.unit[i])
        return 0;
    }
    return fitness == b.fitness;
  }
  unit_instance unit[N];
  int fitness;
};

individual base_pattern = {
  { // unit_instance unit[N]
    {params + UNIT_CRYSTAL_0, 1, 5},
    {params + UNIT_CRYSTAL_0, 2, 3},
    {params + UNIT_CRYSTAL_0, 3, 6},
    {params + UNIT_CRYSTAL_0, 3, 7},
    {params + UNIT_CRYSTAL_0, 5, 2},
    {params + UNIT_CRYSTAL_0, 6, 2},
    {params + UNIT_POWER_PLANT_12, 2, 8},  //1
    {params + UNIT_POWER_PLANT_12, 3, 2},  //2
    {params + UNIT_POWER_PLANT_12, 3, 3},  //3
    {params + UNIT_POWER_PLANT_12, 3, 4},  //4
    {params + UNIT_POWER_PLANT_12, 4, 1},  //5
    {params + UNIT_POWER_PLANT_12, 4, 3},  //6
    {params + UNIT_POWER_PLANT_12, 4, 4},  //7
    {params + UNIT_POWER_PLANT_12, 4, 5},  //8
    {params + UNIT_POWER_PLANT_12, 4, 8},  //9
    {params + UNIT_POWER_PLANT_12, 6, 3},  //10
    {params + UNIT_POWER_PLANT_12, 7, 3},  //11
    {params + UNIT_POWER_PLANT_15, 7, 1},
    {params + UNIT_POWER_PLANT_16, 2, 7},  //1
    {params + UNIT_POWER_PLANT_16, 4, 6},  //2
    {params + UNIT_POWER_PLANT_16, 4, 7},  //3
    {params + UNIT_POWER_PLANT_16, 5, 1},  //4
    {params + UNIT_POWER_PLANT_16, 5, 3},  //5
    {params + UNIT_POWER_PLANT_16, 6, 1},  //6
    {params + UNIT_POWER_PLANT_17, 2, 4},
    {params + UNIT_POWER_PLANT_17, 2, 5},
    {params + UNIT_POWER_PLANT_19, 2, 6},
    {params + UNIT_ACCUMULATOR_12, 3, 8},
    {params + UNIT_ACCUMULATOR_12, 6, 4},
    {params + UNIT_ACCUMULATOR_12, 7, 2},
    {params + UNIT_ACCUMULATOR_19, 4, 2},
    {params + UNIT_ACCUMULATOR_20, 3, 5},
  },
  0
};

char *base = "..........t...c.t....c......t....cc............c.t.tt...c...............";

individual *population_current = new individual[M];
individual *population_next    = new individual[M];
individual *individual_ptr     = NULL;

individual best_list[10];

const int BUFFER_SIZE = 80 * 24;

char buffer[BUFFER_SIZE];

int count = 0;

inline int max(int a, int b) { return a > b ? a : b; }
//int abs(int x) { return x > 0 ? x : -x; }


inline int L1dist(const unit_instance &a, const unit_instance &b) {
  return max(abs(a.row - b.row), abs(a.col - b.col));
}

inline int rand30() {
  return ((rand() << 15) | rand());
}


void CheckConsistence(char *str) {
  //printf("%s\n", str);
  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < N; ++j) {
      int x = population_current[i].unit[j].row + population_current[i].unit[j].col;
      if (!(0<= x && x <= 20)){
        printf("i=%d j=%d x=%d\n", i, j, x);
      }
      //assert(0 <= x && x <= 20);
    }
  }
}



int calc_fitness(individual *candidate) {
  static int cnt = 0;
  //printf("calc_fitness call %d\n", cnt);
  //CheckConsistence("repro_out");
  int valid_list[N];
  memset(valid_list, 0, sizeof valid_list);
  //CheckConsistence("repro_in");
  for (int i = C; i < N; ++i) {
    int coord = candidate->unit[i].row * 9 + candidate->unit[i].col;
    //if (coord >= 72) {
    //  printf("%d\n", coord);
    //}
    //assert(coord < 72);
    char t = base[coord];
    if (t != '.')
      valid_list[i] = 1;
    for (int j = i + 1; j < N; ++j) {
      if (candidate->unit[i].row == candidate->unit[j].row && candidate->unit[i].col == candidate->unit[j].col) {
        valid_list[j] = 1;
        //printf("%d %d", i, j);
      }
    }
  }
  int sum = 0;
  for (int i = C; i < N; ++i) {
    if (valid_list[i])
      continue;
    int c, a1 = 0, a2 = 0;
    c = candidate->unit[i].param->constant;
    if (candidate->unit[i].param->type == UNIT_POWER_PLANT) {
      
      for (int j = 0; j < C; ++j) {
        if (valid_list[j]) 
          continue;
        if (L1dist(candidate->unit[i], candidate->unit[j]) == 1)
          a1 += candidate->unit[i].param->acc1;
      }
      for (int j = C; j < N; ++j) {
        if (valid_list[j]) 
          continue;
        if (L1dist(candidate->unit[i], candidate->unit[j]) == 1 && candidate->unit[j].param->type == UNIT_ACCUMULATOR)
          a2 = candidate->unit[i].param->acc2;
      }
    } else if (candidate->unit[i].param->type == UNIT_ACCUMULATOR) {
      for (int j = C; j < N; ++j) {
        if (valid_list[j]) 
          continue;
        if (L1dist(candidate->unit[i], candidate->unit[j]) == 1 && candidate->unit[j].param->type == UNIT_POWER_PLANT)
          a1 += candidate->unit[i].param->acc1;
      }
    }
    sum += c + a1 + a2;
  }
  //for (int i = 0; i < N; ++i)
  // if (valid_list[i])  
  //sum = 1;
  return sum;
}

int random_selectioin(int exclude) {
  int idx = 0, val = 0;
  for (int i = 0; i < M; ++i) {
    int v = population_current[i].fitness / 100 * (rand() % 100 + 1);
    if (v > val && i != exclude) {
      val = v;
      idx = i;
    }
  }
  return idx;
}


inline void swap(unit_instance &a, unit_instance &b) {
  unit_instance t = a;
  a = b;
  b = t;
}

void shift(int k) {
  for (int i = k + 1; i < 10; ++i) {
    best_list[i] = best_list[i - 1];
  }
}

inline void update_list(individual *x) {
  if (x->fitness > best_list[0].fitness)
    best_list[0] = *x;
}



void reproduction(individual *a, individual *b) {
  individual *c = individual_ptr++;
  *c = *a;

  int pos = C + rand() % (N - C) - 2;
  
  for (int i = C; i <= pos; ++i) {
    c->unit[i] = b->unit[i];
    // assert(c->unit[i] == b->unit[i]);
  }
  

  // mutate
  for (int i = C; i < N; ++i) {
    //int p = 10;
    //if (i > 26)
    //  p = 3;
    if (rand() % 10 == 0) {
      if (rand() % 2) {
        c->unit[i].row = rand() % 8;
        c->unit[i].col = rand() % 9;
      } else {
        int x = rand() % (N - C);
        //assert(x >= 0);
        //assert(x + C < N);
        int tr, tc;
        tr = c->unit[i].row;
        tc = c->unit[i].col;
        
        c->unit[i].row = c->unit[C + x].row;
        c->unit[i].col = c->unit[C + x].col;

        //assert(0 <= tr && tr< 8);
        //assert(0 <= tc && tc< 9);

        c->unit[C + x].row = tr;
        c->unit[C + x].col = tc;
        
      }
    }
    //if (rand() % 1000 == 0) {
    //  d->unit[i].row = rand() % 8;
    //  d->unit[i].col = rand() % 9;
    //}
  }
  //CheckConsistence("repro_in");
  c->fitness = calc_fitness(c);
  //d->fitness = calc_fitness(d);

  // update best list
  update_list(c);
  //update_list(d);
}


inline void draw_pixel(int r, int c, char v) {
  buffer[r * 80 + c] = v;
}

void print_individual(individual *a) {
  memset(buffer, ' ', sizeof buffer);
  unit_instance u;
  for (int i = 0; i < N; ++i) {
    u = a->unit[i];
    if (u.param->type == UNIT_CRYSTAL) {
      draw_pixel(u.row, u.col, 'c');
      //buffer[u.row * 9 + u.row + 1] = 'C';
    } else if (u.param->type == UNIT_ACCUMULATOR) {
      draw_pixel(u.row, u.col, '*');
    } else if (u.param->type == UNIT_POWER_PLANT) {
      draw_pixel(u.row, u.col, '#');
    }
  }
  
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 9; ++j) {
      if (base[i * 9 + j] == 't')
        draw_pixel(i, j, 't');
    }
  }
  sprintf(buffer + 8 * 80, "iteration: %d", count);
  sprintf(buffer + 9 * 80, "best: %d", a->fitness);
  for (int i = 0; i < sizeof(buffer) - 1; ++i)
    if (buffer[i] == '\0')
      buffer[i] = ' ';
  buffer[BUFFER_SIZE - 1] = 0;
  printf("%s\n", buffer);
}



void random_individual(const individual *pattern, individual *dst) {
  for (int i = 0; i < C; ++i) {
    dst->unit[i] = pattern->unit[i];
  }
  
  for (int i = C; i < N; ++i) {
    dst->unit[i].param = pattern->unit[i].param;
    dst->unit[i].row = rand() % 8;
    dst->unit[i].col = rand() % 9;
  }
  
}

void init(int k) {
  for (int i = k; i < M; ++i) {
    random_individual(&base_pattern, &population_current[i]);
    population_current[i].fitness = calc_fitness(&population_current[i]);
  }
  //CheckConsistence("123");
}

void GA() {
  init(0);
  //CheckConsistence("init");
  //return;
  for(;;) {
    individual_ptr = population_next;
    int i = 0;
    while (individual_ptr - population_next < M) {
      int x = random_selectioin(-1);
      //CheckConsistence("rand1");
      int y = random_selectioin(x);
      //CheckConsistence("rand2");
      //printf("%d %d\n", x, y);
      reproduction(&population_current[x], &population_current[y]);
      //CheckConsistence("repro");
      //printf("%d\n", i++);
    }
    individual *t_ptr = population_current;
    population_current = population_next;
    population_next = t_ptr;
    if (best_list[0].fitness > 0)
      print_individual(&best_list[0]);
    ++count;
    if (count % 10 == 0)
      init(M/4);
    //if (count % 50)
    //  init(0);
    //system("PAUSE");
  }

  
}




int main()
{
  srand(time(0));
  GA();
  return 0;
}

