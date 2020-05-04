/*
    Copyright: Yongping Luo  Email: ypluo18@qq.com
*/

#ifndef __BUCKET_H__
#define __BUCKET_H__

namespace extendible_hash {

using std::vector;

constexpr int BUCKET_SIZE = 3;

template<typename _key_t, typename _value_t>
struct Record {
    _key_t key;
    _value_t value;
};

template<typename _key_t, typename _value_t, bool ISCHAIN>
struct BlkChain {
    typedef BlkChain<_key_t, _value_t, ISCHAIN> blkchain_t;
    typedef Record<_key_t, _value_t> record_t;

    int freepos;
    blkchain_t * next_bucket;
    record_t recs[BUCKET_SIZE];
    
    BlkChain() {
        freepos = 0; 
        next_bucket = NULL;
    }

    ~BlkChain() {
        if(next_bucket != NULL)
            delete next_bucket;
    }

    inline bool insert(const _key_t key, const _value_t val) {
        if(freepos >= BUCKET_SIZE) {
            if(ISCHAIN == false) {
                return false; // can not insert into the bucket
            }   
            
            if(next_bucket == NULL) { // has no overflow block
                next_bucket = new blkchain_t;
            }
            return next_bucket->insert(key, val);
            
        } else {
            recs[freepos++] = {key, val};
            return true;
        }        
    }

    inline bool find(const _key_t key, _value_t & val) const {
        for(int i = 0; i < freepos; i++) { // find in current block
            if(recs[i].key == key) {
                val = recs[i].value;
                return true;
            }
        }

        if(ISCHAIN == true && next_bucket != NULL) { // find in the next block
            return next_bucket->find(key, val);
        }

        return false;
    }

    inline bool remove(const _key_t key) {
        for(int i = 0; i < freepos; i++) {
            if(recs[i].key == key) {
                if(i != freepos - 1) { // move a key to fill the empty slot
                    recs[i] = recs[freepos - 1];
                }

                freepos -= 1;
                return true;
            }
        }

        if(ISCHAIN == true && next_bucket != NULL) {
            return next_bucket->remove(key);
        }

        return false;
    }

    record_t & operator [] (int idx) {
        if(idx < freepos) {
            return recs[idx];
        } else if(ISCHAIN == true && next_bucket != NULL){
            return next_bucket->operator[](idx - freepos);
        } else {
            assert(idx < freepos); // can not be here
        }
    }

    void print() {
        for(int i = 0; i < freepos; i++) {
            printf(" %d ", recs[i].key);
        }

        if(ISCHAIN == true && next_bucket != NULL) {
            next_bucket->print();
        }
    }
};


template<typename _key_t, typename _value_t, bool ISCHAIN=false>
struct Bucket: public BlkChain<_key_t, _value_t, ISCHAIN> {
    typedef Bucket<_key_t, _value_t, ISCHAIN> self_t;
    typedef BlkChain<_key_t, _value_t, ISCHAIN> blkchain_t;
    
    size_t len;
    int bkt_bits;
    
    Bucket(int bits = 0){
        len = 0;
        bkt_bits = bits;
    }

    inline bool insert(const _key_t key, const _value_t val) {
        bool success = blkchain_t::insert(key, val);

        if(success)
            len++;

        return success;      
    }

    inline bool remove(const _key_t key) {
        bool success = blkchain_t::remove(key);

        if(success)
            len--;

        return success;      
    }

    size_t size() const {
        return len;
    }

    void print() {
        printf(">[");
        blkchain_t::print();
        printf("]\n");
    }
};

};  //namespace extendible_hash

#endif //__BUCKET_H__