/*
    Copyright: Yongping Luo  Email: ypluo18@qq.com
*/

#ifndef __DOUBLE_EXTHASH_H__
#define __DOUBLE_EXTHASH_H__

#include <vector>
#include <cstdint>
#include <cassert>

#include "bucket.h"

namespace extendible_hash {

template<typename _key_t, typename _value_t>
class Double {
    public:
        typedef Bucket<_key_t, _value_t, false> bucket_t;

        typedef Record<_key_t, _value_t> record_t;

    private:
        uint8_t global_bits;

        vector<bucket_t *> bkts;

        inline int locate_bucket(const _key_t key) {
            // may use the hashvalue of key to do the locate
            return key >> (sizeof(_key_t) * 8 - 1 - global_bits);
        }

        inline int get_bit(const _key_t key, int bit) { // key must be integer
            // may use the hashvalue of key
            return (key & (1 << (sizeof(_key_t) * 8 - 1 - bit))) > 0 ? 1 : 0;
        }

        void split_bucket(bucket_t * from, bucket_t *to) {
            // split one bucket into two of it
            from->bkt_bits++;
            to->bkt_bits = from->bkt_bits;

            vector<record_t> should_move;
            // get records to move
            for(int i = 0; i < from->size(); i++) {
                record_t rec = (*from)[i];
                if(get_bit(rec.key, from->bkt_bits) == 1) {
                    should_move.push_back(rec);
                }
            }

            // insert records to new bucket
            for(int i = 0; i < should_move.size(); i++) {
                to->insert(should_move[i].key, should_move[i].value);
            }

            // delete records in the origin bucket
            for(int i = should_move.size() - 1; i >= 0; i--) {
                from->remove(should_move[i].key);
            }
        }

    public:
        Double(int start_bits = 0) {
            global_bits = start_bits;

            bkts.push_back(new bucket_t);
            bkts.push_back(new bucket_t);

            bkts[0]->bkt_bits = global_bits;
            bkts[1]->bkt_bits = global_bits;
        }

        ~Double() {
            for(int i = 0; i < bkts.size() - 1; i++) {
                if(bkts[i] == bkts[i + 1]) {
                    continue;
                }
                delete bkts[i];
            }

            delete bkts[bkts.size() - 1];
        }

        void insert(const _key_t key, const _value_t val) {
            // locate the bucket
            int cur_idx = locate_bucket(key);
            bucket_t * to_buck = bkts[cur_idx];

            if(to_buck->insert(key, val) == false) { // the bucket is full

                if((cur_idx > 0 && bkts[cur_idx - 1] == to_buck) || 
                    (cur_idx < bkts.size() && bkts[cur_idx + 1] == to_buck)) {// we only need to split the bucket
                    int start_idx = cur_idx;
                    int end_idx = cur_idx; // find all the bkts that point to the same bucket as to_buck
                    
                    while (start_idx > 0 && bkts[start_idx - 1] == to_buck) 
                        start_idx -= 1;
                    
                    while (end_idx < bkts.size() - 1 && bkts[end_idx + 1] == to_buck) 
                        end_idx += 1;

                    bucket_t * new_bkt = new bucket_t;
                    
                    split_bucket(to_buck, new_bkt);

                    for(int i = (start_idx + end_idx + 1) / 2; i <= end_idx; i++) {
                        bkts[i] = new_bkt;
                    }
                } else { // need to extend the hashtable
                    vector<bucket_t *> new_bkts;

                    for(int i = 0; i < bkts.size(); i++) {
                        bucket_t * cur_bkt = bkts[i];

                        if(i == cur_idx) {
                            bucket_t * split_bkt = new bucket_t;
                            split_bucket(cur_bkt, split_bkt);

                            new_bkts.push_back(cur_bkt);
                            new_bkts.push_back(split_bkt);  
                        } else {
                            new_bkts.push_back(cur_bkt);
                            new_bkts.push_back(cur_bkt);
                        }
                    }
                    bkts.swap(new_bkts);

                    global_bits += 1;
                }

                insert(key, val);
            }
        }

        bool find(const _key_t key, _value_t & val) {
            // locate the bucket
            int cur_idx = locate_bucket(key);
            bucket_t * to_buck = bkts[cur_idx];

            return to_buck->find(key, val);
        }

        bool remove(const _key_t key) {
            // locate the bucket
            int cur_idx = locate_bucket(key);
            bucket_t * to_buck = bkts[cur_idx];

            return to_buck->remove(key);
        }

        void print() {
            for(int i = 0; i < bkts.size() - 1; i++) {
                printf("Buckets %d --", i);
                
                if(bkts[i] == bkts[i + 1]) {
                    printf("|\n");
                    continue;
                } else {
                    bkts[i]->print();
                }   
            }
            
            printf("Buckets %d --", bkts.size() - 1);
            bkts[bkts.size() - 1]->print();
        }
};

}; //namespace extendible_hash

#endif //__DOUBLE_EXTHASH_H__