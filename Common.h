// 2022-11-11 Common functions

struct Range
{
    uint32_t begin = 0;
    uint32_t end = 0;
};

template <typename T>
class Span
{
    T* begin_ = nullptr;
    T* end_ = nullptr;

public:
    Span() = default;
    Span(const Span&) = default;
    Span(Span&&) = default;

    template<typename U>
    Span(Span<U>&& u) : begin_(u.begin()), end_(u.end())
    {
        // Ensure no accidental type slicing from child class to base class,
        // which would access the wrong memory for later elements.
        static_assert(sizeof(*u.begin()) == sizeof(*begin_));
    }

    Span(T* p, size_t s) : begin_(p), end_(begin_ + s) {}
    Span(T* b, T* e) : begin_(b), end_(e) {}

    const T* data() const noexcept { return begin_; }
    T* data() noexcept { return begin_; }
    size_t size() const noexcept { return end_ - begin_; }
    T* begin() const noexcept { return begin_; }
    T* end() const noexcept { return end_; }
    T* begin() noexcept { return begin_; }
    T* end() noexcept { return end_; }
    T& operator [](size_t index) noexcept { return begin_[index]; }
    const T& operator [](size_t index) const noexcept { return begin_[index]; }
    T& front() noexcept { return *begin_; }
    const T& front() const noexcept { return *begin_; }
    T& back() noexcept { return *(end_ - 1); }
    const T& back() const noexcept { return *(end_ - 1); }
    bool empty() const noexcept { return begin_ == end_; }

    void PopFront() { ++begin_; }
    void PopBack() { ++end_; }
};

template <typename FullType, typename SmallType>
struct SmallEnum
{
    SmallEnum(FullType t = {}) : value(static_cast<SmallType>(t)) {}
    operator FullType() {return static_cast<FullType>(value);}

    SmallType value;
};

template <typename ContainerType>
auto MakeSpan(ContainerType& container) -> Span<std::remove_reference_t<decltype(*container.data())> >
{
    using T = std::remove_reference_t<decltype(*container.data())>;
    return Span<T>(container.data(), container.size());
}
