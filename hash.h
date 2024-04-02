#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include <stdlib.h>

#define TABLE_SIZE 100

// 哈希节点结构
typedef struct Node {
  unsigned int key;
  uint64_t value;
  struct Node *next;
} Node;

// 哈希表结构
typedef struct {
  Node *table[TABLE_SIZE];
} HashMap;

// 哈希函数，简单取模
unsigned int hash(unsigned int key);

// 初始化哈希表
HashMap *initHashMap();

// 插入键值对
void insert(HashMap *map, unsigned int key, int value);

// 查找键对应的值
uint64_t get(HashMap *map, unsigned int key);
// 销毁哈希表
void destroyHashMap(HashMap *map);

#endif // HASH_H