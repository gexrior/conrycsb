#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <stdint.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <malloc.h>
#include <common.hpp>

using namespace std;


template<typename KeyType, typename DataType>
struct SkipListNode
{
    //add:xurui
//    VersionInfo version_infos[MAX_VERSION_NUM];
//    atomic<int> ref_count;
//    atomic<int> table_id;
    //add:e
    pair<KeyType,DataType> keyvalue;
    SkipListNode **next; //指向一个指针数组的指针
    //add:xurui
    atomic<int> rf_count;
    bool mark;

    int inc_ref()
    {
        return __sync_add_and_fetch(&rf_count, 1);
    }
    int dec_ref()
    {
        return __sync_sub_and_fetch(&rf_count, 1);
    }

    //add:e
};


template<typename KeyType, typename DataType>
class SkipList
{
    public:
        static const int MAXLEVEL=10;  //链表最大高度
        typedef SkipListNode<KeyType,DataType> Node;
        typedef pair<KeyType,DataType> KeyValue;

        SkipList();
        void clear();
        bool Insert(const KeyValue& value);
        int getRandomLevel();
        Node* Find(KeyType key) const;
        Node* Find1(KeyType key) const;
        Node* Get(KeyType key) const;
        bool Scan(KeyType start_key, KeyType end_key, vector<Node*> &nodes) const;
        bool Delete(KeyType key);
//        bool Delete(Range range);

    protected:

    public:
        Node* create_node(KeyValue value, int level);
//        void detroy_node(Node *node);

        Node *head_;
        int level_; //当前链表高度
};


template<typename KeyType,typename DataType>
SkipList<KeyType,DataType>::SkipList()
{
    head_ = create_node(std::make_pair(KeyType(),DataType()),MAXLEVEL);
    level_ = 0;
}

template<typename KeyType,typename DataType>
void SkipList<KeyType,DataType>::clear()
{
    //dtor
 //   free(mem_base_address_);
 //   mem_base_address_=NULL;
 //   curr_base_address_=NULL;
    head_ = NULL;
    level_ = 0;
}

template<typename KeyType,typename DataType>
SkipListNode<KeyType,DataType>* SkipList<KeyType,DataType>::create_node(KeyValue value,int level)
{
    Node* node=new Node();
    node->next= new Node*[level];

    node->keyvalue.first = value.first;
    node->keyvalue.second = value.second;

    for(int i=0;i<level;i++)
    {
        node->next[i]=NULL;
    }
    //add:xurui
    node->rf_count = 0;
    node->mark =false;
    //add:e
    return node;
}

//template<typename KeyType,typename DataType>
//bool SkipList<KeyType,DataType>::Insert(const KeyValue& value)
//{
//    Node* node=head_;
//    Node* update[MAXLEVEL];
//    //1.find previous node
//    for(int i=level_-1;i>=0;i--)
//    {
//        while(node->next[i]!=NULL)
//        {
//            if(node->next[i]->keyvalue.first < value.first)
//            {
//                node=node->next[i];
//            }
//            else if(node->next[i]->keyvalue.first == value.first)
//            {
//                cout<<"we had this key."<<endl;
//                return false;
//            }
//            else
//            {
//                break;
//            }
//        }
//        update[i]=node;
//    }
//    int newlevel=getRandomLevel();
//    printf("newlevel=%d\n",newlevel);
//    if(newlevel>level_)
//    {
//        for(int i=level_;i<newlevel;i++)
//        {
//            update[i]=head_;
//        }
//        level_=newlevel;
//    }
//    //2.create node
//    node=create_node(value,newlevel);
//    //3.insert node
//    for(int i=0;i<newlevel;i++)
//    {
//        node->next[i]=update[i]->next[i];
//        update[i]->next[i]=node;
//    }

//    return true;
//}

template<typename KeyType,typename DataType>
int SkipList<KeyType,DataType>::getRandomLevel()
{
    float p=0.25;
    int level=1;
    while((rand()&0xFFFF) < (p * 0xFFFF))
    {
        level++;
    }
    return (level<MAXLEVEL) ? level : MAXLEVEL;
}

//template<typename KeyType,typename DataType>
//SkipListNode<KeyType,DataType>* SkipList<KeyType,DataType>::Find(KeyType key) const
//{
//    Node* node=head_;
//    for(int i=level_-1;i>=0;i--)
//    {
//        while(node->next[i]!=NULL)
//        {
//            if(node->next[i]->keyvalue.first<key)
//            {
//                node=node->next[i];
//            }
//            else if(node->next[i]->keyvalue.first==key)
//            {
//                return node->next[i];
//            }
//            else
//            {
//                break;
//            }
//        }
//    }

//    return NULL;
//}

template<typename KeyType,typename DataType>
SkipListNode<KeyType,DataType>* SkipList<KeyType,DataType>::Find(KeyType key) const
{
    Node* pre=head_;

    pre->inc_ref();
    //add:e
    Node* curr=head_->next[level_-1];

    curr->inc_ref();
    //add:e
    for(int i=level_-1;i>=0;i--)
    {
        while(curr!=NULL)
        {
            if(curr->keyvalue.first<key)
            {
//                printf("node->next[i]->keyvalue.first<key;\n");

                pre->dec_ref();
                //add:e
                pre = curr;
                //curr->dec_ref();
                curr=curr->next[i];
            }
            else if(curr->keyvalue.first==key)
            {
                pre->dec_ref();
                if(curr->mark == false)
                {
                    return curr;
                }
                else //node has been deleted
                {
                    curr->dec_ref();
                    return NULL;
                }
                //add:e

//                printf("node->next[i]->keyvalue.first==key;\n");
//                return curr;
            }
            else
            {

                curr->dec_ref();
                //add:e
                curr=pre->next[i-1];

//                printf("break;\n");
                break;
            }
        }

        if(curr == NULL && i-1>=0)
        {

            curr->dec_ref();
            //add:e
            curr == pre->next[i-1];
        }
    }

    pre->dec_ref();
    //add:e

    return NULL;
}

template<typename KeyType,typename DataType>
SkipListNode<KeyType,DataType>* SkipList<KeyType,DataType>::Find1(KeyType key) const
{
    Node* pre=head_;
    Node* curr=head_->next[level_-1];
    for(int i=level_-1;i>=0;i--)
    {
        printf("i=%d\n",i);
        while(curr!=NULL)
        {
            if(curr->keyvalue.first<key)
            {
                printf("node->next[i]->keyvalue.first<key;\n");
                pre = curr;
                curr=curr->next[i];
            }
            else if(curr->keyvalue.first==key)
            {
                if(curr==head_->next[0])
                {
                   printf("curr==head_->next[0];\n");
                }
                printf("node->next[i]->keyvalue.first==key;\n");
                return curr;
            }
            else
            {
                curr=pre->next[i-1];
                printf("break;\n");
                break;
            }
        }

        if(curr == NULL && i-1>=0)
        {
            printf("curr=NULL;\n");
            curr == pre->next[i-1];
        }
    }

    return NULL;
}

//add:modified by huangcc
template<typename KeyType,typename DataType>
bool SkipList<KeyType,DataType>::Insert(const KeyValue& value)
{
    Node* pre=head_;
    pre->inc_ref();
    //add:e
    Node* curr=head_->next[level_-1];
    curr->inc_ref();
    //add:e
    Node* update[MAXLEVEL];
    //1.find previous node
    for(int i=level_-1;i>=0;i--)
    {
        while(curr!=NULL)
        {
            if(curr->keyvalue.first < value.first)
            {

                pre->dec_ref();
                //add:e
                pre = curr;
                //curr->dec_ref();
                curr=curr->next[i];
            }
            else if(curr->keyvalue.first == value.first)
            {
                cout<<"we had this key."<<endl;
                //add:xurui
                pre->dec_ref();
                curr->dec_ref();
                //add:e
                return false;
            }
            else
            {

                curr->dec_ref();
                //add:e
                curr= pre->next[i-1];
                break;
            }
        }

        if(curr == NULL && i-1>=0)
        {

            curr->dec_ref();
            //add:e
            curr == pre->next[i-1];
        }

        update[i]=pre;
    }

    pre->dec_ref();
    //add:e

    int newlevel=getRandomLevel();
//    printf("newlevel=%d\n",newlevel);
    if(newlevel>level_)
    {
        for(int i=level_;i<newlevel;i++)
        {
            update[i]=head_;
        }
        level_=newlevel;
    }
    //2.create node
    Node* node=create_node(value,newlevel);
    //3.insert node
    for(int i=0;i<newlevel;i++)
    {
        node->next[i]=update[i]->next[i];
        update[i]->next[i]=node;
    }

    return true;
}
//add:e


//template<typename KeyType,typename DataType>
//SkipListNode<KeyType,DataType>* SkipList<KeyType,DataType>::Get(KeyType key) const
//{
//    Node* node=head_;
//    for(int i=level_-1;i>=0;i--)
//    {
//        while(node->next[i]!=NULL)
//        {
//            if(node->next[i]->keyvalue.first<key)
//            {
//                printf("node->next[i]->keyvalue.first<key;\n");
//                node=node->next[i];
//            }
//            else if(node->next[i]->keyvalue.first==key)
//            {
//                printf("node->next[i]->keyvalue.first==key;\n");
//                return node->next[i];
//            }
//            else if(i==0)//?
//            {
//                printf("i == 0;\n");
//                return node;
//            }
//            else
//            {
//                printf("break;\n");
//                break;
//            }
//        }
//    }

//    return NULL;
//}

//add:modified by huangcc
template<typename KeyType,typename DataType>
SkipListNode<KeyType,DataType>* SkipList<KeyType,DataType>::Get(KeyType key) const
{
    Node* pre=head_;

    pre->inc_ref();
    //add:e
    Node* curr=head_->next[level_-1];

    curr->inc_ref();
    //add:e
    for(int i=level_-1;i>=0;i--)
    {
        while(curr!=NULL)
        {
            if(curr->keyvalue.first<key)
            {
//                printf("node->next[i]->keyvalue.first<key;\n");

                pre->dec_ref();
                //add:e
                pre = curr;
                //curr->dec_ref();
                curr=curr->next[i];
            }
            else if(curr->keyvalue.first==key)
            {

                pre->dec_ref();
                if(curr->mark == false)
                {
                    return curr;
                }
                else //node has been deleted
                {
                    curr->dec_ref();
                    return NULL;
                }
                //add:e
//                printf("node->next[i]->keyvalue.first==key;\n");
//                return curr;
            }
            else if(i==0)//?
            {
//                printf("i == 0;\n");
                return pre;
            }
            else
            {

                curr->dec_ref();
                //add:e
                curr=pre->next[i-1];
//                printf("break;\n");
                break;
            }
        }

        if(curr == NULL)
        {
            if(i>0)
            {

                curr->dec_ref();
                //add:e
                curr = pre->next[i-1];
            }
            else
            {
                return curr;
            }

        }
    }

    return NULL;
}
//add:e
//add:modified by huangcc
template<typename KeyType,typename DataType>
bool SkipList<KeyType,DataType>::Scan(KeyType start_key, KeyType end_key, vector<Node*> &nodes) const
{
    Node* node = NULL;
    node = Get(start_key);
    if(node == NULL)
    {
        printf("no find!\n");
        return false;
    }
    else
    {
//        printf("find!\n");
        if(node != head_)
        {
            nodes.push_back(node);
        }

        //printf("1\n");
    //    printf("size=%d",(int)nodes.size());

        //delete huangcc 20191129
        while(node->next[0]!=NULL)
        {
            if(node->next[0]->keyvalue.first == end_key)
            {
//                nodes[count++] = node->next[0];
                //nodes.push_back(node->next[0]);
//                printf("test1\n");
                if(nodes.size()<SCAN_LENGTH)
                {
                   nodes.push_back(node->next[0]);
                }
                break;
            }
            else if(node->next[0]->keyvalue.first < end_key)
            {
//                nodes[count++] = node->next[0];
//                nodes.push_back(node->next[0]);
//                node=node->next[0];
//                  printf("test2\n");
                  if(nodes.size()<SCAN_LENGTH)
                  {
                      nodes.push_back(node->next[0]);
                      node=node->next[0];
                  }
                  else
                  {
                      break;
                  }

            }
            else
            {
                break;
            }
        }
        //delete
        return true;
    }

}
//add:e
template<typename KeyType,typename DataType>
bool SkipList<KeyType,DataType>::Delete(KeyType key)
{
    Node* node=head_;

    node->inc_ref();
    //add:e
    Node* update[level_];
    int key_level=0;

    for(int i=level_-1;i>=0;i--)
    {
        while(node->next[i]!=NULL)
        {
            if(node->next[i]->keyvalue.first < key)
            {

                node->dec_ref();
                //add:e
                node=node->next[i];
            }
            else if(node->next[i]->keyvalue.first == key)
            {

                key_level++;
                if(key_level == 1)
                {
                    node->next[i]->mark = true;
                }
                node->next[i]->dec_ref();
                break;
                //计算key节点next指针一共有多少层
//                key_level=sizeof(node->next[i]->next)/sizeof(Node*);
//                break;
            }
            else
            {

                node->next[i]->dec_ref();
                //add:e
                break;
            }
        }
        update[i]=node;
    }
    node->dec_ref();

    if(key_level == 0)
    {
        return false;  //没有找到节点
    }
    else
    {
        Node* p = update[0]->next[0];
        for(int i=0; i<key_level; i++)
        {
            update[i]->next[i]=update[i]->next[i]->next[i];

        }
        delete p->next;
        p->next = NULL;
 //       delete p->keyvalue;
 //       p->keyvalue = NULL;
        delete p;
        p = NULL;
    }

    return true;
}

#endif // SKIPLIST_H
