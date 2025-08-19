// #ifdef SENTINEL
// #error "SENTINEL is defined!"
// #endif

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
 
    nil->left = nil;
    nil->right = nil;
    nil->parent = nil;

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

// Case1) p = red, u = red			: p,u = black, pp = red 위로반복
// Case2) p = red, u = black + tri	: rotate -> Case3
// Case3) p = red, u = black + lst	: 색깔 변경 + rotate
// RBtree 특성상 삽입하는 노드는 무조건 Red (5번조건)
// RBtree 특성상 삽입하는 위치는 노드 (BST기반) -> p = black을 안따져도됨

static void insert_fixup(rbtree* tree, node_t* node)
{
    if (!tree || node == tree->nil) return;

    while (node->parent->color == RBTREE_RED)
    {
        // node의 부모가 왼쪽 자식일 때
        if (node->parent == node->parent->parent->left)
        {
            //           [pp(B)]
            //      [p(R)]     [u(R)]
            //  [n(R)]

            node_t* u = node->parent->parent->right; // 삼촌 노드

            if (u->color == RBTREE_RED)
            {   // case1:  p = red, u = red	
                node->parent->color = RBTREE_BLACK;
                u->color = RBTREE_BLACK;
                node->parent->parent->color = RBTREE_RED;
                node = node->parent->parent;
            }
            else // case2,3: p = red, u = black
            {
                // Case 2 (삼각형)
                // Triangle------------------------------------
                //           [pp(B)]
                //      [p(R)]     [u(B)]
                //          [n(R)] 

                if (node == node->parent->right)
                { 
                    node = node->parent;
                    left_rotate(tree, node); // Case 3가 됨
                }

                // Case 3 (리스트)
                // List------------------------------------
                //           [pp(B)]
                //      [p(R)]     [u(B)]
                // [n(R)]

                //           [p(B)]
                //      [n(R)]     [pp(R)]
                //                      [u(B)]
                node->parent->color = RBTREE_BLACK;
                node->parent->parent->color = RBTREE_RED;
                right_rotate(tree, node->parent->parent);
            }
        }
        else // z의 부모가 오른쪽 자식일 때 (위에꺼 대칭적으로 옮기셈)
        { 
            //           [pp(B)]
            //      [u(R)]     [p(R)]
            //                      [n(R)]

            node_t* u = node->parent->parent->left;

            if (u->color == RBTREE_RED)
            {   // case1:  p = red, u = red	
                node->parent->color = RBTREE_BLACK;
                u->color = RBTREE_BLACK;
                node->parent->parent->color = RBTREE_RED;
                node = node->parent->parent;
            }
            else //case2,3: p = red, u = black
            {
                // Case 2 (삼각형)
                // Triangle------------------------------------
                //           [pp(B)]
                //      [u(B)]      [p(R)]
                //              [n(R)]

                if (node == node->parent->left)
                { 
                    node = node->parent;
                    right_rotate(tree, node); // Case 3가 됨
                }

                // Case 3 (리스트)
                // List-----------------------------------
                //           [pp(B)]
                //      [u(B)]      [p(R)]
                //                       [n(R)]

                //               [p(B)]
                //          [pp(R)]     [n(R)]
                //      [u(B)]
                node->parent->color = RBTREE_BLACK;
                node->parent->parent->color = RBTREE_RED;
                left_rotate(tree, node->parent->parent);
            }
        }
    }
    tree->root->color = RBTREE_BLACK; // 루트는 항상 검정색 유지
}


node_t* rbtree_insert(rbtree* tree, const key_t key)
{
    node_t* node = (node_t*)calloc(1, sizeof(node_t));

    node->key = key;
    node->color = RBTREE_RED;
    node->left = tree->nil;
    node->right = tree->nil;
    node->parent = tree->nil;

    // 삽입 위치를 찾기위함
    //              [y]
    //            /
    //  [x(node)]
    node_t* y = tree->nil;
    node_t* x = tree->root;

    while (x != tree->nil)
    {
        y = x;
        if (node->key < x->key)
        {
            x = x->left;
        }
        else
        {
            x = x->right;
        }
    }

    node->parent = y;

    if (y == tree->nil)
    {
        tree->root = node;
    }
    else if (node->key < y->key)
    {
        y->left = node;
    }
    else
    {
        y->right = node;
    }

    node->left = tree->nil;
    node->right = tree->nil;
    insert_fixup(tree, node);

    return node;
}

/*
 * 트리에서 key 값으로 노드 검색
 * BST 탐색 방식으로 진행
 */
node_t* rbtree_find(const rbtree* tree, const key_t key)
{
    if (!tree) return NULL;

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
    if (!tree) return NULL;
    node_t* now = tree->root;

    if (now == tree->nil) return tree->nil;

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
    if (!tree) return NULL;
    node_t* now = tree->root;

    if (now == tree->nil) return tree->nil;

    while (now->right != tree->nil)
    {
        now = now->right;
    }

    return now;
}

/*
 * 삭제 대체 노드(석섹서) 찾기
 * 오른쪽 서브트리의 최소값 or 부모로 올라가서 최초로 왼쪽 자식이 아닌 부모를 찾음
 */
static node_t* tree_successor(const rbtree* tree, node_t* node)
{
    if (!tree || node == tree->nil) return NULL;

    // 1. node의 오른쪽 서브트리가 있으면
    if (node->right != tree->nil)
    {
        node_t* now = node->right;

        while (now->left != tree->nil)
        {
            now = now->left;
        }

        return now;
    }

    // 2. 부모 쪽으로 올라가면서, node가 부모의 왼쪽 자식이 되는 순간의 부모가 석세서  
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
 * Case 1) 삭제할 노드가 red : 걍 삭제함(erase_fixup을 적용안함)
 * Case 2) Root가 DB : 추가 black 삭제
 * Case 3) DB의 형제가 red : s=black, p=red (s와 p 색상교환) + DB방향으로 Rotate(p) => 다른 Case로 전환
 * Case 4) DB의 형제가 black && 형제의 양쪽 자식 모두 black : DB를 parent에게 이전, (p = !p_color), s=red => 아직 DB가 있다면 p를 대상으로 알고리즘 실행
 * Case 5) DB의 형제가 black && 형제의 near자식=red, 형제의 far자식=black : near=black, s=red(s와 near의 색상교환) + far방향으로 Rotate(s) => Case 6
 * Case 6) DB의 형제가 black && 형제의 far자식=red => far=black, (p와 s의 색상교환) + DB방향으로 Rotate(p) => 추가 black 삭제
 */
static void erase_fixup(rbtree* tree, node_t* x)
{
	  // 삭제할 노드 x는 DB상태이다
 
    // Case 2) 루트가 DB면, 루프 조건에서 바로 종료되어 x->color만 black으로 변경됨

    while (x != tree->root && x->color == RBTREE_BLACK)
    {
        // 삭제하려는 것이 왼쪽 자식일때
        if (x == x->parent->left)
        {
            // s = sibling
            node_t* s = x->parent->right;

            // Case 3) s가 red이면 색상 교환 후 p 기준으로 좌회전
            if (s->color == RBTREE_RED)
            { 
                //            [p(B)]
                //     [x(DB)]      [s(R)]
                //               [sl]     [sr]
                //   
                //            [s(B)]
                //       [p(R)]     [sr]
                //  [x(DB)]   [sl*]

                s->color = RBTREE_BLACK;
                x->parent->color = RBTREE_RED;
                left_rotate(tree, x->parent);
                s = x->parent->right; // *바뀐 형제 갱신
            }
        
            // Case 4) s와 s의 자식 둘 다 black이면 s를 red로, x의 DB를 부모로 올림
            if (s->left->color == RBTREE_BLACK && s->right->color == RBTREE_BLACK)
            { 
                //             [p]
                //     [x(DB)]      [s(B)]
                //             [sl(B)]   [sr(B)]
                //
                //            [p(DB)]
                //      [x(B)]        [s(R)]
                //               [sl(B)]   [sr(B)]
                
                s->color = RBTREE_RED;
                x = x->parent;
            }
 
            // Case 5) s=black, sr=black, sl=red면 색상 교환 후 s 기준으로 우회전 -> Case 6로
            else 
            {
                if (s->right->color == RBTREE_BLACK)
                { 
                    //             [p]
                    //     [x(DB)]      [s(B)]
                    //             [sl(R)]   [sr(B)]
                    //
                    //             [p]
                    //     [x(DB)]      [sl(B)*] 
                    //                       [s(R)]
                    //                           [sr(B)]
                    //
                    s->left->color = RBTREE_BLACK;
                    s->color = RBTREE_RED;
                    right_rotate(tree, s);
                    s = x->parent->right; // *바뀐 형제 갱신
                }
               
                // Case 6) s=black, sr=red면, s와p 색깔 교환, p 기준으로 좌회전, DB 제거
                
                //            [p(R)]
                //     [x(DB)]    [s(B)]
                //                     [sr(R)]
                //
                //              [s(R)]
                //        [p(B)]       [sr(B)]
                //    [x(B)]
                
                s->color = x->parent->color;
                x->parent->color = RBTREE_BLACK;
                s->right->color = RBTREE_BLACK;
                left_rotate(tree, x->parent);
                x = tree->root; // DB 루트로 보내서 제거
            }
        }
        else // 오른쪽 자식일때(위에꺼 대칭)
        {
            node_t* s = x->parent->left;

            // Case 3) s가 red이면 색상 교환 후 p 기준으로 우회전
            if (s->color == RBTREE_RED)
            {
                //            [p(B)]
                //      [s(R)]        [x(DB)]
                //   [sl]   [sr]
                //
                //            [s(B)]
                //       [sl]       [p(R)]
                //               [sr*]   [x(DB)]

                s->color = RBTREE_BLACK;
                x->parent->color = RBTREE_RED;
                right_rotate(tree, x->parent);
                s = x->parent->left; // *바뀐 형제 갱신
            }

            // Case 4) s와 s의 자식 둘 다 black이면 s를 red로, x의 DB를 부모로 올림
            if (s->right->color == RBTREE_BLACK && s->left->color == RBTREE_BLACK)
            {
                //                [p]
                //        [s(B)]        [x(DB)]
                //   [sl(B)]   [sr(B)]
                //
                //              [p(DB)]
                //         [s(R)]        [x(B)]
                //   [sl(B)]   [sr(B)]

                s->color = RBTREE_RED;
                x = x->parent;
            }
            else
            {
                // Case 5) s=black, sl=black, sr=red면 색상 교환 후 s 기준으로 좌회전 -> Case 6로
                if (s->left->color == RBTREE_BLACK)
                {
                    //                  [p]
                    //         [s(B)]        [x(DB)]
                    //   [sl(B)]   [sr(R)]
                    //
                    //                  [p]
                    //         [sr(B)*]      [x(DB)]
                    //     [s(R)]   [sl(B)]
                    //
                    s->right->color = RBTREE_BLACK;
                    s->color = RBTREE_RED;
                    left_rotate(tree, s);
                    s = x->parent->left; // *바뀐 형제 갱신
                }

                // Case 6) s=black, sl=red면 p 기준으로 우회전, DB 제거

                //                [p]
                //        [s(B)]        [x(DB)]
                //   [sl(R)]   
                //
                //             [s(R)]
                //      [sl(B)]      [p(B)]
                //                       [x(B)]

                s->color = x->parent->color;
                x->parent->color = RBTREE_BLACK;
                s->left->color = RBTREE_BLACK;
                right_rotate(tree, x->parent);
                x = tree->root; // DB 루트로 보내서 제거
            }
        }
    }

    // Case 1) 삭제할 노드가 red : 걍 삭제함 (x가 red면 fixup 호출 자체를 안함)
    x->color = RBTREE_BLACK;
}

/*
 * 1. 삭제 대상 노드(node)가 자식이 0개 또는 1개면 node가 직접 삭제됨.
 *    -> 자식이 2개면 석세서를 찾아 node의 key값만 교체하고 node_erase를 삭제.
 * 2. 삭제를 위해 node_erase의 자식(x)와 부모 연결을 갱신.
 * 3. 실제 삭제는 항상 자식이 0개 또는 1개인 노드(y)에서 발생
 */
int rbtree_erase(rbtree* tree, node_t* node)
{
    if (!tree || node == tree->nil) return 0;

    // node_erase: 실제로 삭제될 노드
    node_t* node_erase = (node->left == tree->nil || node->right == tree->nil) ? node : tree_successor(tree, node);

    // x: node_erase의 자식, node_erase를 대체할 녀석
    node_t* x = (node_erase->left != tree->nil) ? node_erase->left : node_erase->right;

    // x의 부모를 node_erase의 부모로 연결
    x->parent = node_erase->parent;

    // 트리의 root가 삭제되는 경우 root를 x로 교체
    if (node_erase->parent == tree->nil)
    {
        tree->root = x;
    }
    else if (node_erase == node_erase->parent->left)
    {
        node_erase->parent->left = x;
    }
    else
    {
        node_erase->parent->right = x;
    }

    // 삭제 대상이 석세서(node_erase)라면 node의 key값만 node_erase의 key로 복사
    if (node_erase != node)
    {
        node->key = node_erase->key;
    }

    // 삭제된 노드가 BLACK면 재조정
    if (node_erase->color == RBTREE_BLACK)
    {
        erase_fixup(tree, x);
    }

    free(node_erase);
    return 0;
}

/*
 * 중위순회를 하면 오름차순 배열로 변하니까
 */
static void to_array_inorder(const rbtree* tree, node_t* node, key_t* arr, size_t* i, size_t n_size)
{
    if (node == tree->nil) return;

    to_array_inorder(tree, node->left, arr, i, n_size);

    if (*i < n_size)
    {
        // 여기서 arr[*i++]로 해버린 실수
        arr[(*i)++] = node->key;
    }

    to_array_inorder(tree, node->right, arr, i, n_size);
}

/*
 * 트리 전체를 key 오름차순 배열로 변환
 */
int rbtree_to_array(const rbtree* tree, key_t* arr, const size_t n)
{
    size_t i = 0;
    to_array_inorder(tree, tree->root, arr, &i, n);
    return (int)i;
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

void print_array(const key_t* arr, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}


// int main(void)
// {
//     // 트리 생성 체크
//     rbtree* tree = new_rbtree();
//     print_rbtree(tree);

//     // insert 체크
//     rbtree_insert(tree, 30);
//     print_rbtree(tree);

//     rbtree_insert(tree, 10);
//     print_rbtree(tree);

//     rbtree_insert(tree, 20);
//     print_rbtree(tree);

//     rbtree_insert(tree, 25);
//     print_rbtree(tree);

//     printf("\n");

//     // find 체크
//     node_t* find = rbtree_find(tree, 20);
//     if (find != tree->nil)
//     {
//         printf("find: ");
//         printf("%d\n", find->key);
//     }
//     else {
//         printf("not found\n");
//     }

//     // min 체크
//     node_t* min_ = rbtree_min(tree);
//     printf("min: ");
//     printf("%d\n", min_->key);

//     // max 체크
//     node_t* max_ = rbtree_max(tree);
//     printf("max: ");
//     printf("%d\n", max_->key);

//     // 다음 노드 체크
//     node_t* next = tree_successor(tree, find);
//     printf("next of %d: ", find->key);
//     printf("%d\n", next->key);

//     // 배열 테스트
//     key_t arr[100];
//     int array = rbtree_to_array(tree, arr, 100);
//     printf("\n arr: ");
//     print_array(arr, array);

//     // 삭제 테스트
//     node_t* del_node = rbtree_find(tree, 20);
//     if (del_node)
//     {
//         rbtree_erase(tree, del_node);
//         print_rbtree(tree);
//     }
//     del_node = rbtree_find(tree, 10);
//     if (del_node)
//     {
//         rbtree_erase(tree, del_node);
//         print_rbtree(tree);
//     }

//     // 트리 삭제 체크
//     delete_rbtree(tree);
//     print_rbtree(tree);
//     return 0;
// }

// 1. test_init
// 목적 : 레드블랙트리의 초기화가 올바른지 확인.
// 동작 :
// 트리를 새로 만들고
// 트리 자체 / 루트 / 센티넬(nil) 노드가 제대로 초기화되었는지 체크
// 트리 삭제
// 
// 2. test_insert_single
// 목적 : 단일 노드 삽입이 올바르게 동작하는지 확인.
// 동작 :
// 트리에 하나의 key를 삽입
// 루트가 해당 노드인지, 값 / 포인터 / 색깔이 정상인지 체크
// 트리 삭제
// 
// 3. test_find_single
// 목적 : 트리에서 key를 찾는 기능이 제대로 동작하는지 확인.
// 동작 :
// 하나의 key를 삽입
// 올바른 key로 찾으면 해당 노드가 반환되는지
// 없는 key로 찾으면 NULL 반환되는지 체크
// 트리 삭제
// 
// 4. test_erase_root
// 목적 : 루트 노드 삭제가 제대로 이루어지는지 확인.
// 동작 :
// 하나의 key를 삽입
// 루트 노드를 삭제
// 삭제 후 트리 루트가 nil 또는 NULL인지 체크
// 트리 삭제
// 
// 5. test_find_erase_fixed
// 목적 : 여러 값에 대해 삽입 / 검색 / 삭제가 반복적으로 정상 동작하는지 확인.
// 동작 :
// 고정된 배열의 key들을 모두 트리에 삽입
// 각 키를 찾아서 삭제, 삭제 후 다시 찾아서 없는지 확인
// 다시 삽입 / 검색 / 삭제를 반복
// 트리 삭제
// 
// 6. test_minmax_suite
// 목적 : 트리의 최소값 / 최대값 반환 및 삭제 후 동작이 올바른지 확인.
// 동작 :
// 여러 값을 삽입 후,
// 트리에서 최소 / 최대값을 찾아 반환값이 올바른지 체크
// 삭제 후에도 min / max가 제대로 갱신되는지 확인
// 트리 삭제
// 
// 7. test_to_array_suite
// 목적 : 트리의 중위순회 결과가 오름차순 정렬 배열과 같은지 확인.
// 동작 :
// 여러 값을 트리에 삽입
// 트리를 배열로 변환
// 배열과 오름차순 정렬된 원본 배열을 비교
// 트리 삭제
// 
// 8. test_distinct_values
// 목적 : 중복 없는(모두 다른) 값을 잘 관리하는지, 트리 속성과 색상 규칙을 지키는지 확인.
// 동작 :
// 여러 서로 다른 값을 삽입
// 트리의 레드블랙트리 속성(색상 / 검색 규칙 등) 검증
// 트리 삭제
// 
// 9. test_duplicate_values
// 목적 : 중복된 값들이 삽입될 때도 레드블랙트리 규칙이 유지되는지 확인.
// 동작 :
// 중복값이 포함된 배열을 삽입
// 트리의 색상 / 검색 규칙 등 레드블랙트리 속성 검증
// 트리 삭제
// 
// 10. test_multi_instance
// 목적 : 여러 레드블랙트리를 동시에 관리할 때 각각의 트리가 독립적으로 잘 동작하는지 확인.
// 동작 :
// 두 개의 트리를 생성
// 각각에 배열을 삽입 / 정렬 / 배열 변환
// 각 트리의 결과가 독립적으로 올바른지 확인
// 트리 삭제
// 
// 11. test_find_erase_rand
// 목적 : 대량의 랜덤 값에 대해 삽입 / 검색 / 삭제가 반복적으로 정상 동작하는지 검증(스트레스 테스트).
// 동작 :
// 랜덤 시드로 큰 배열을 생성, 삽입 / 검색 / 삭제 반복
// 모든 값이 올바르게 삽입 / 삭제 / 검색되는지 확인
// 트리 삭제