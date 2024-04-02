
#include "hash.h"

// 哈希函数，简单取模
unsigned int hash(unsigned int key) { return key % TABLE_SIZE; }

// 初始化哈希表
HashMap *initHashMap() {
  HashMap *map = (HashMap *)malloc(sizeof(HashMap));
  for (int i = 0; i < TABLE_SIZE; i++) {
    map->table[i] = NULL;
  }
  return map;
}

// 插入键值对
void insert(HashMap *map, unsigned int key, int value) {
  unsigned int index = hash(key);
  Node *newNode = (Node *)malloc(sizeof(Node));
  newNode->key = key;
  newNode->value = value;
  newNode->next = NULL;

  if (map->table[index] == NULL) {
    map->table[index] = newNode;
  } else {
    Node *current = map->table[index];
    while (current->next != NULL) {
      current = current->next;
    }
    current->next = newNode;
  }
}

// 查找键对应的值
uint64_t get(HashMap *map, unsigned int key) {
  unsigned int index = hash(key);
  Node *current = map->table[index];
  while (current != NULL) {
    if (current->key == key) {
      return current->value;
    }
    current = current->next;
  }
  // 如果没有找到，返回一个默认值，比如 -1
  return 0;
}

// 销毁哈希表
void destroyHashMap(HashMap *map) {
  for (int i = 0; i < TABLE_SIZE; i++) {
    Node *current = map->table[i];
    while (current != NULL) {
      Node *temp = current;
      current = current->next;
      free(temp);
    }
  }
  free(map);
}
