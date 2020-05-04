/*
    Copyright: Yongping Luo  Email: ypluo18@qq.com
*/

#ifndef __LINEAR_EXTHASH_H__
#define __LINEAR_EXTHASH_H__

#include <vector>
#include <cstdint>
#include <cassert>

#include "bucket.h"

namespace extendible_hash {

template<typename _key_t, typename _value_t>
class Linear {
    public:
        typedef Bucket<_key_t, _value_t, true> bucket_t;

        typedef Record<_key_t, _value_t> record_t;

    private:
        float threshold;
        uint8_t global_bits;

        uint32_t total_size;
        int split_idx;
        vector<bucket_t *> bkts;

        inline int locate_bucket(const _key_t key) {
            // may use the hashvalue of key to do the locate
            int idx = key & (((uint64_t)1 << (global_bits + 1)) - 1);

            if(idx < split_idx) {
                return key & (((uint64_t)1 << (global_bits + 2)) - 1);
            } else {
                return idx;
            }
        }

        inline int get_bit_rev(const _key_t key, int bit) { // key must be integer
            // may use the hashvalue of key
            return (key & (1 << bit)) > 0 ? 1 : 0;
        }

        void split_bucket(bucket_t * from, bucket_t *to) {
            // split one bucket into two of it
            vector<record_t> should_move;
            // get records to move
            for(int i = 0; i < from->size(); i++) {
                record_t rec = (*from)[i];
                if(get_bit_rev(rec.key, global_bits + 1) == 1) {
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
        Linear(float thres = 0.8): threshold(thres){
            global_bits = 0;
            split_idx = 0;

            total_size = 0;
            bkts.push_back(new bucket_t);
            bkts.push_back(new bucket_t);
        }

        ~Linear() {
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

            to_buck->insert(key, val);

            total_size += 1;

            if((float)total_size / (bkts.size() * BUCKET_SIZE) > threshold) { // we need to split a bucket
                bucket_t * new_bucket = new bucket_t;

                split_bucket(bkts[split_idx], new_bucket);

                bkts.push_back(new_bucket);

                // split_idx
                if(split_idx < ((uint64_t)1 << (global_bits + 1)) - 1) {
                    split_idx += 1;
                } else {
                    split_idx = 0;
                    global_bits += 1;
                }
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

#endif //__LINEAR_EXTHASH_H__