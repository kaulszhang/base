// LimitNumber.h

#ifndef _FRAMEWORK_SYSTEM_LIMIT_NUMBER_H_
#define _FRAMEWORK_SYSTEM_LIMIT_NUMBER_H_

namespace framework
{
    namespace system
    {

        template <
            size_t N, 
            typename Ty = boost::uint64_t
        >
        class LimitNumber
        {
        public:
            LimitNumber(
                Ty n = 0)
                : test_bit_((Ty)1 << (N - 1))
                , carry_bit_((Ty)1 << N)
                , low_bits_(n)
                , high_bits_(0)
            {
                assert(n < carry_bit_);
            }
 
        public:
            Ty transfer(
                Ty n)
            {
                assert(n < carry_bit_);
                if ((low_bits_ ^ n) & test_bit_) {
                    if (low_bits_ & test_bit_) {
                        high_bits_ += carry_bit_;
                    }
                }
                low_bits_ = n;
                return low_bits_ | high_bits_;
            }
 
        private:
            Ty test_bit_;
            Ty carry_bit_;
            Ty low_bits_;
            Ty high_bits_;
        };

    }
}

#endif // _FRAMEWORK_SYSTEM_LIMIT_NUMBER_H_

