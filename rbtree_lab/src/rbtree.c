#include "rbtree.h"
#include <stdlib.h>
#include <stdio.h>


/*
 * 새 레드블랙트리 생성
 * nil 생성하여 root와 nil을 초기화
 * 모든 삽입/삭제에서 nil 노드를 사용함
 */
rbtree* new_rbtree(void)
{
    rbtree* tree = (rbtree*)calloc(1, sizeof(rbtree));
    node_t* nil = (node_t*)calloc(1, sizeof(node_t));
    nil->color = RBTREE_BLACK;
    nil->left = NULL;
    nil->right = NULL;
    nil->parent = NULL;
    tree->nil = nil;
    tree->root = tree->nil;
    return tree;
}

/*
 * 트리의 노드 삭제(후위순회)
 */
static void delete_node(rbtree* tree, node_t* node)
{
    if (node == tree->nil) return;

    delete_node(tree, node->left);
    delete_node(tree, node->right);
    free(node);
}


/*
 * 트리 및 nil 노드 메모리 해제
 */
void delete_rbtree(rbtree* tree)
{
    if (!tree) return;

    delete_node(tree, tree->root);
    free(tree->nil);
    free(tree);
}

//////////////////////////////////////////////////////////////////////////////////////////

/*
 * 트리 재조정을 위한 회전(좌, 우)
 */
static void left_rotate(rbtree* tree, node_t* x)
{

    //            [p]
    //        [x]
    //    [1]     [y]
    //          [2] [3]
    //----------------------
    //            [p]
    //        [y]
    //    [x]     [3]
    //  [1] [2]

     // [y]
    node_t* y = x->right;

    // [2]
    x->right = y->left;
    if (y->left != tree->nil)
    {
        y->left->parent = x;
    }

    // [p]
    y->parent = x->parent;
    if (x->parent == tree->nil)
    {
        // 애초에 x가 root였다면
        tree->root = y;
    }
    else if (x == x->parent->left)
    {
        //       [p]
        //    [x]
        x->parent->left = y;
    }
    else
    {
        //      [p]
        //          [x]
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

static void right_rotate(rbtree* tree, node_t* y)
{
    //           [p]
    //        [y]
    //    [x]     [3]
    //  [1] [2]
    //----------------------
    //           [p]
    //        [x]
    //    [1]     [y]
    //          [2] [3]

    // [x]
    node_t* x = y->left;

    // [2]
    y->left = x->right;
    if (x->right != tree->nil)
    {
        x->right->parent = y;
    }

    // [p]
    x->parent = y->parent;
    if (y->parent == tree->nil)
    {
        // 애초에 x가 root였다면
        tree->root = x;
    }

    else if (y == y->parent->right)
    {
        //      [p]
        //         [x]
        y->parent->right = x;
    }
    else
    {
        //      [p]
        //  [x]
        y->parent->left = x;
    }

    x->right = y;
    y->parent = x;
}

/*
 * 삽입 후 재조정
 * Case 1: 부모(p)와 삼촌(u)이 모두 RED인 경우, 부모와 삼촌을 BLACK, 조부모(pp)를 RED로 바꾼 뒤 node를 조부모로 옮겨 반복
 * Case 2: 부모는 RED, 삼촌은 BLACK이며 node가 '삼각형' 구조인 경우, 회전시켜 Case 3으로 만듦
 * Case 3: 부모는 RED, 삼촌은 BLACK이며 node가 '리스트' 구조인 경우, 색상 변경 후 회전
 * 마지막에 루트는 항상 BLACK으로 유지.
 */
static void insert_fixup(rbtree* tree, node_t* node)
{
    while (node->parent->color == RBTREE_RED)
    {
        // node의 부모가 왼쪽 자식일 때
        if (node->parent == node->parent->parent->left)
        {
            node_t* y = node->parent->parent->right; // 삼촌 노드
            if (y->color == RBTREE_RED)
            { // Case 1
                node->parent->color = RBTREE_BLACK;
                y->color = RBTREE_BLACK;
                node->parent->parent->color = RBTREE_RED;
                node = node->parent->parent;
            }
            else
            {
                if (node == node->parent->right)
                { // Case 2 (삼각형)
                    node = node->parent;
                    left_rotate(tree, node);
                }

                // Case 3 (일직선)
                node->parent->color = RBTREE_BLACK;
                node->parent->parent->color = RBTREE_RED;
                right_rotate(tree, node->parent->parent);
            }
        }
        else
        { // z의 부모가 오른쪽 자식일 때 (위에꺼 대칭적으로 옮기셈)
            node_t* y = node->parent->parent->left;
            if (y->color == RBTREE_RED)
            { // Case 1
                node->parent->color = RBTREE_BLACK;
                y->color = RBTREE_BLACK;
                node->parent->parent->color = RBTREE_RED;
                node = node->parent->parent;
            }
            else {
                if (node == node->parent->left)
                { // Case 2 (삼각형)
                    node = node->parent;
                    right_rotate(tree, node);
                }

                // Case 3 (리스트)
                node->parent->color = RBTREE_BLACK;
                node->parent->parent->color = RBTREE_RED;
                left_rotate(tree, node->parent->parent);
            }
        }
    }
    tree->root->color = RBTREE_BLACK; // 루트는 항상 검정색 유지
}

/*
 * 트리 삽입
 * BST 삽입 후 색상은 RED로, left/right/nil 연결
 * 삽입 후 insert_fixup으로 재조정
 */
node_t* rbtree_insert(rbtree* tree, const key_t key)
{
    node_t* z = (node_t*)calloc(1, sizeof(node_t));

    z->key = key;
    z->color = RBTREE_RED;
    z->left = z->right = z->parent = tree->nil;

    node_t* y = tree->nil;
    node_t* x = tree->root;
    while (x != tree->nil)
    {
        y = x;
        if (z->key < x->key)
        {
            x = x->left;
        }
        else
        {
            x = x->right;
        }
    }

    z->parent = y;
    if (y == tree->nil)
    {
        tree->root = z;
    }
    else if (z->key < y->key)
    {
        y->left = z;
    }
    else
    {
        y->right = z;
    }

    z->left = z->right = tree->nil;
    insert_fixup(tree, z);
    return z;
}

/*
 * 트리에서 key 값으로 노드 검색
 * BST 탐색 방식으로 진행
 */
node_t* rbtree_find(const rbtree* tree, const key_t key)
{
    node_t* now = tree->root;
    while (now != tree->nil)
    {
        if (key == now->key)
        {
            return now;
        }
        now = (key < now->key ? now->left : now->right);
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////

/*
 * 트리에서 최소값 노드 반환.
 * 왼쪽으로 쭉
 */
node_t* rbtree_min(const rbtree* tree)
{
    node_t* now = tree->root;

    if (now == tree->nil) return NULL;

    while (now->left != tree->nil)
    {
        now = now->left;
    }

    return now;
}

/*
 * 트리에서 최대값 노드 반환.
 * 오른쪽으로 쭉
 */
node_t* rbtree_max(const rbtree* tree)
{
    node_t* now = tree->root;

    if (now == tree->nil) return NULL;

    while (now->right != tree->nil)
    {
        now = now->right;
    }

    return now;
}

/*
 * 삭제 대체 노드(후속 노드) 찾기
 * 오른쪽 서브트리의 최소값 또는 부모로 올라가서 최초로 왼쪽 자식이 아닌 부모를 찾음
 */
static node_t* tree_successor(const rbtree* tree, node_t* node)
{
    if (node->right != tree->nil)
    {
        node_t* now = node->right;

        while (now->left != tree->nil)
        {
            now = now->left;
        }

        return now;
    }

    node_t* y = node->parent;

    while (y != tree->nil && node == y->right)
    {
        node = y;
        y = y->parent;
    }

    return y;
}

/*
 * 삭제 후 트리 규칙 복구
 * Case 1~6: 형제, 조부모, 자식의 색과 구조에 따라
 * 색상 변경 및 회전 반복
 */
static void erase_fixup(rbtree* tree, node_t* x)
{
    while (x != tree->root && x->color == RBTREE_BLACK)
    {
        if (x == x->parent->left)
        {
            node_t* w = x->parent->right;
            if (w->color == RBTREE_RED)
            { // Case 3
                w->color = RBTREE_BLACK;
                x->parent->color = RBTREE_RED;
                left_rotate(tree, x->parent);
                w = x->parent->right;
            }
            if (w->left->color == RBTREE_BLACK && w->right->color == RBTREE_BLACK)
            { // Case 4
                w->color = RBTREE_RED;
                x = x->parent;
            }
            else {
                if (w->right->color == RBTREE_BLACK)
                { // Case 5
                    w->left->color = RBTREE_BLACK;
                    w->color = RBTREE_RED;
                    right_rotate(tree, w);
                    w = x->parent->right;
                }
                // Case 6
                w->color = x->parent->color;
                x->parent->color = RBTREE_BLACK;
                w->right->color = RBTREE_BLACK;
                left_rotate(tree, x->parent);
                x = tree->root;
            }
        }
        else
        {
            node_t* w = x->parent->left;
            if (w->color == RBTREE_RED)
            { // Case 3(대칭)
                w->color = RBTREE_BLACK;
                x->parent->color = RBTREE_RED;
                right_rotate(tree, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == RBTREE_BLACK && w->left->color == RBTREE_BLACK)
            { // Case 4(대칭)
                w->color = RBTREE_RED;
                x = x->parent;
            }
            else
            {
                if (w->left->color == RBTREE_BLACK)
                { // Case 5(대칭)
                    w->right->color = RBTREE_BLACK;
                    w->color = RBTREE_RED;
                    left_rotate(tree, w);
                    w = x->parent->left;
                }
                // Case 6(대칭)
                w->color = x->parent->color;
                x->parent->color = RBTREE_BLACK;
                w->left->color = RBTREE_BLACK;
                right_rotate(tree, x->parent);
                x = tree->root;
            }
        }
    }
    x->color = RBTREE_BLACK;
}

/*
 * 트리에서 노드 삭제
 * 자식이 하나 또는 둘일 때 각각 대체 노드(x)로 교체
 * y의 색이 BLACK이면 erase_fixup으로 트리 규칙 복구
 */
int rbtree_erase(rbtree* tree, node_t* node)
{
    node_t* y = (node->left == tree->nil || node->right == tree->nil) ? node : tree_successor(tree, node);
    node_t* x = (y->left != tree->nil) ? y->left : y->right;

    x->parent = y->parent;
    if (y->parent == tree->nil)
    {
        tree->root = x;
    }
    else if (y == y->parent->left)
    {
        y->parent->left = x;
    }
    else
    {
        y->parent->right = x;
    }


    if (y != node)
    {
        node->key = y->key;
    }


    if (y->color == RBTREE_BLACK)
    {
        erase_fixup(tree, x);
    }


    free(y);
    return 0;
}

/*
 * 트리를 오름차순으로 배열에 저장(중위순회)
 * n_size를 넘지 않는 범위에서 값을 복사
 */
static void to_array_rec(const rbtree* tree, node_t* node, key_t* arr, size_t* i, size_t n_size)
{
    if (node == tree->nil) return;

    to_array_rec(tree, node->left, arr, i, n_size);

    if (*i < n_size)
    {
        arr[(*i)++] = node->key;
    }

    to_array_rec(tree, node->right, arr, i, n_size);
}

/*
 * 트리 전체를 key 오름차순 배열로 변환
 */
int rbtree_to_array(const rbtree* tree, key_t* arr, const size_t n)
{
    size_t i = 0;
    to_array_rec(tree, tree->root, arr, &i, n);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void print_rbtree_rec(const rbtree* tree, node_t* node, int depth, char branch) {
    if (node == tree->nil) return;
    print_rbtree_rec(tree, node->right, depth + 1, '/');

    for (int i = 0; i < depth; ++i) printf("    ");
    printf("%c-", branch);
    printf("%d(%s)\n", node->key, node->color == RBTREE_RED ? "R" : "B");

    print_rbtree_rec(tree, node->left, depth + 1, '\\');
}

void print_rbtree(const rbtree* tree) {
    printf("\n[RBTree]\n");
    print_rbtree_rec(tree, tree->root, 0, '|');
}

//  int main(void)
//  {
//      rbtree* tree = new_rbtree();

//      // 삽입 테스트
//      rbtree_insert(tree, 30);
//      print_rbtree(tree);

//      rbtree_insert(tree, 10);
//      print_rbtree(tree);

//      rbtree_insert(tree, 20);
//      print_rbtree(tree);

//      rbtree_insert(tree, 25);
//      print_rbtree(tree);

//      // 삭제 테스트
//      node_t* del_node = rbtree_find(tree, 20);
//      if (del_node) 
//      {
//          rbtree_erase(tree, del_node);
//          print_rbtree(tree);
//      }
    
//      del_node = rbtree_find(tree, 10);
//      if (del_node) 
//      {
//          rbtree_erase(tree, del_node);
//          print_rbtree(tree);
//      }

//      // 마무리
//      delete_rbtree(tree);

//      return 0;
//  }