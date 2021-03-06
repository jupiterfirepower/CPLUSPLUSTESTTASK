#include <mutex>
#include <thread>
#include <condition_variable>
#include <climits>

class shared_mutex_base {
        public:
            shared_mutex_base() = default;
            shared_mutex_base(const shared_mutex_base&) = delete;
            ~shared_mutex_base() = default;

            shared_mutex_base& operator = (const shared_mutex_base&) = delete;

        protected:
            using unique_lock = std::unique_lock<std::recursive_mutex>;
            using scoped_lock = std::lock_guard<std::recursive_mutex>;

            std::recursive_mutex m_mutex;
            std::condition_variable_any m_exclusive_release;
            std::condition_variable_any m_shared_release;
            unsigned m_state = 0;

            void do_exclusive_lock(unique_lock& lk);
            bool do_exclusive_trylock(unique_lock& lk);
            void do_lock_shared(unique_lock& lk);
            bool do_try_lock_shared(unique_lock& lk);
            void do_unlock_shared(scoped_lock& lk);

            void take_exclusive_lock();
            bool someone_has_exclusive_lock() const;
            bool no_one_has_any_lock() const;
            unsigned number_of_readers() const;
            bool maximal_number_of_readers_reached() const;
            void clear_lock_status();
            void increment_readers();
            void decrement_readers();

            static const unsigned m_write_entered = 1U << (sizeof(unsigned)*CHAR_BIT - 1);
            static const unsigned m_num_readers = ~m_write_entered;
        };

    /// <summary> This is a non-standard class which is essentially the same as `shared_mutex` but
    /// it allows a thread to recursively obtain write locks as long as the unlock count matches
    /// the lock-count. </summary>
    class recursive_shared_mutex : public shared_mutex_base {
    public:
        recursive_shared_mutex() = default;
        recursive_shared_mutex(const recursive_shared_mutex&) = delete;
        ~recursive_shared_mutex() = default;

        recursive_shared_mutex& operator = (const recursive_shared_mutex&) = delete;

        /// <summary> Obtains an exclusive lock of this mutex. For recursive calls will always obtain the
        /// lock. </summary>
        void lock();

        /// <summary> Attempts to exclusively lock this mutex. For recursive calls will always obtain the
        /// lock. </summary>
        /// <returns> true if it the lock was obtained, false otherwise. </returns>
        bool try_lock();

        /// <summary> Unlocks the exclusive lock on this mutex. </summary>
        void unlock();

        /// <summary> Obtains a shared lock on this mutex. Other threads may also hold a shared lock simultaneously. </summary>
        void lock_shared();

        /// <summary> Attempts to obtain a shared lock for this mutex. </summary>
        /// <returns> true if it the lock was obtained, false otherwise. </returns>
        bool try_lock_shared();

        /// <summary> Unlocks the shared lock on this mutex. </summary>
        void unlock_shared();

        /// <summary> Number recursive write locks. </summary>
        /// <returns> The total number of write locks. </returns>
        int num_write_locks();

        /// <summary> Query if this object is exclusively locked by me. </summary>
        /// <returns> true if locked by me, false if not. </returns>
        bool is_locked_by_me();

    private:
        std::thread::id m_write_thread;
        int m_write_recurses = 0;
    };