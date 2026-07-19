#ifndef CPX_INFERENCE_TENSOR_H
#define CPX_INFERENCE_TENSOR_H

#include <cpx/tag_info.h>
#include <cpx/reflect.h>
#include <stdexcept>
#include <vector>
#include <variant>

namespace cpx::inference {
    /// Represents a tensor with shape, element type, and raw data.
    CPX_EXPORT struct Tensor {

        /* types */

        /// Supported tensor element types.
        enum class ElementType {
            dynamic, //!< Dynamic element type
            boolean, //!< Boolean element type
            bf16,    //!< bfloat16 element type
            f16,     //!< IEEE float16 element type
            f32,     //!< IEEE float32 element type
            f64,     //!< IEEE float64 element type
            i4,      //!< Signed 4-bit integer element type
            i8,      //!< Signed 8-bit integer element type
            i16,     //!< Signed 16-bit integer element type
            i32,     //!< Signed 32-bit integer element type
            i64,     //!< Signed 64-bit integer element type
            u1,      //!< Unsigned 1-bit integer element type
            u2,      //!< Unsigned 2-bit integer element type
            u3,      //!< Unsigned 3-bit integer element type
            u4,      //!< Unsigned 4-bit integer element type
            u6,      //!< Unsigned 6-bit integer element type
            u8,      //!< Unsigned 8-bit integer element type
            u16,     //!< Unsigned 16-bit integer element type
            u32,     //!< Unsigned 32-bit integer element type
            u64,     //!< Unsigned 64-bit integer element type
            nf4,     //!< NormalFloat-4 quantized element type
            f8e4m3,  //!< Float8 (E4M3) element type
            f8e5m2,  //!< Float8 (E5M2) element type
            string,  //!< String element type
            f4e2m1,  //!< Float4 (E2M1) element type
            f8e8m0,  //!< Float8 (E8M0) element type
        };

        /// Compile-time known tensor dimension.
        using StaticDimension = size_t;

        /// Runtime tensor dimension, either a fixed size or symbolic name.
        using DynamicDimension = std::variant<StaticDimension, std::string>;

        /// Tensor shape with only static dimensions.
        using StaticShape = std::vector<StaticDimension>;

        /// Tensor shape with static and/or symbolic dimensions.
        using DynamicShape = std::vector<DynamicDimension>;


        /* fileds */

        /// Tensor name.
        std::string name;

        /// Tensor shape.
        DynamicShape shape;

        /// Tensor element type.
        ElementType element_type = {};

        /// Raw tensor data.
        std::string_view data;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Tensor::name>         = "name,field_number=1",
            cpx::field<&Tensor::shape>        = "shape,field_number=2",
            cpx::field<&Tensor::element_type> = "element-type,field_number=3",
            cpx::field<&Tensor::data>         = ",field_number=4",
        };


        /* methods: shape */

        /// Sets the tensor shape from a vector of dimensions.
        ///
        /// @tparam T Dimension type convertible to `StaticDimension`.
        /// @param shape Tensor dimensions.
        template <typename T = StaticDimension, typename Enable = std::enable_if_t<std::is_convertible_v<T, StaticDimension>>>
        void set_shape(const std::vector<T> &shape);

        /// Returns the tensor shape as a vector of static dimensions.
        ///
        /// @tparam T Target dimension type.
        /// @return Tensor dimensions.
        template <typename T = StaticDimension, typename Enable = std::enable_if_t<std::is_convertible_v<T, StaticDimension>>>
        std::vector<T> get_shape() const;


        /* methods: raw data */

        /// Returns a pointer to the underlying raw tensor data.
        ///
        /// @return Pointer to the tensor data.
        inline void *get_raw_data() const;

        /// Sets the underlying raw tensor data pointer.
        ///
        /// @param data Pointer to the tensor data.
        /// @note Must be invoked after `set_shape()`.
        inline void set_raw_data(void *data);


        /* methods: data */

        /// Returns the tensor data as a typed pointer.
        ///
        /// @tparam T Arithmetic element type.
        /// @return Pointer to the tensor data.
        template <typename T, typename Enable = std::enable_if_t<std::is_arithmetic_v<T>>>
        const T *get_data() const;

        template <typename T, typename Enable = std::enable_if_t<std::is_arithmetic_v<T>>>
        T *get_data();

        /// Copies the tensor data into a vector.
        ///
        /// @tparam T Arithmetic element type.
        /// @return Tensor data as a vector.
        template <typename T, typename Enable = std::enable_if_t<std::is_arithmetic_v<T>>>
        std::vector<T> get_data_as_vector() const;

        /// Copies data from a vector into the tensor.
        ///
        /// @tparam T Arithmetic element type.
        /// @param data Source tensor data.
        /// @note Must be invoked after `set_shape()`.
        template <typename T, typename Enable = std::enable_if_t<std::is_arithmetic_v<T>>>
        void set_data(const std::vector<T> &data);


        /* methods: size */

        /// Returns the number of tensor elements.
        ///
        /// @return Total number of elements.
        inline size_t get_size() const;

        /// Returns the tensor size in bytes.
        ///
        /// @return Total size in bytes.
        inline size_t get_size_in_bytes() const;

        /// Returns the size of a single tensor element.
        ///
        /// @return Element size in bytes.
        inline size_t get_element_size() const;


        inline void expect(const Tensor &other) const;

        inline static void expect_all(const std::vector<Tensor> &expected_tensors, const std::vector<Tensor> &actual_tensors);
    };
    static_assert(std::is_aggregate_v<Tensor> && std::is_default_constructible_v<Tensor>);

    template <typename T, typename Enable>
    void Tensor::set_shape(const std::vector<T> &shape) {
        const auto size = shape.size();
        this->shape.resize(size, 0ul);
        for (size_t i = 0; i < size; ++i)
            this->shape[i] = StaticDimension(shape[i]);
    }

    template <typename T, typename Enable>
    std::vector<T> Tensor::get_shape() const {
        struct ToStringVisitor {
            std::string operator()(StaticDimension dim) const {
                return std::to_string(dim);
            }
            std::string operator()(const std::string &dim) const {
                return dim;
            }
        };

        struct ToStaticVisitor {
            const DynamicShape &shape;

            StaticDimension operator()(StaticDimension dim) const {
                return dim;
            }
            StaticDimension operator()(const std::string &) const {
                std::string     msg     = "Shape must be static: shape=[";
                ToStringVisitor visitor = {};
                for (auto &dim : shape)
                    msg += std::visit(visitor, dim) + ",";
                msg += "]. Forgot to define static shape?";
                throw std::runtime_error(msg);
            }
        };

        const auto size    = this->shape.size();
        const auto visitor = ToStaticVisitor{this->shape};

        std::vector<T> shape(size);
        for (size_t i = 0; i < size; ++i)
            shape[i] = T(std::visit(visitor, this->shape[i]));

        return shape;
    }

    template <typename T, typename>
    inline const T *Tensor::get_data() const {
        if (sizeof(T) != get_element_size())
            throw std::runtime_error("Mismatch element size. Forgot to define element type?");
        return reinterpret_cast<const T *>(this->data.data());
    }

    template <typename T, typename>
    inline T *Tensor::get_data() {
        if (sizeof(T) != get_element_size())
            throw std::runtime_error("Mismatch element size. Forgot to define element type?");
        return reinterpret_cast<T *>(const_cast<char *>(this->data.data()));
    }

    template <typename T, typename Enable>
    std::vector<T> Tensor::get_data_as_vector() const {
        auto begin = get_data<T>();
        auto end   = begin + get_size();
        return {begin, end};
    }

    template <typename T, typename Enable>
    void Tensor::set_data(const std::vector<T> &data) {
        if (sizeof(T) != get_element_size())
            throw std::runtime_error("Mismatch element size. Forgot to define element type?");

        const auto size = data.size();
        if (size != get_size())
            throw std::runtime_error("Mismatch shape. Forgot to define static shape?");

        this->data = {reinterpret_cast<const char *>(data.data()), size * sizeof(T)};
    }


    inline size_t Tensor::get_size() const {
        size_t res = 1;
        for (auto dim : get_shape())
            res *= dim;
        return res;
    }

    inline size_t Tensor::get_size_in_bytes() const {
        if (auto elem_size = get_element_size(); elem_size != 0)
            return get_size() * elem_size;
        throw std::runtime_error("Element type cannot be dynamic");
    }

    inline size_t Tensor::get_element_size() const {
        switch (element_type) {
        case ElementType::dynamic: return 0;

        case ElementType::boolean:
        case ElementType::i4:
        case ElementType::i8:
        case ElementType::u1:
        case ElementType::u2:
        case ElementType::u3:
        case ElementType::u4:
        case ElementType::u6:
        case ElementType::u8:
        case ElementType::nf4:
        case ElementType::f8e4m3:
        case ElementType::f8e5m2:
        case ElementType::f8e8m0:
        case ElementType::f4e2m1: return 1;

        case ElementType::i16:
        case ElementType::u16:
        case ElementType::bf16:
        case ElementType::f16: return 2;

        case ElementType::i32:
        case ElementType::u32:
        case ElementType::f32: return 4;

        case ElementType::i64:
        case ElementType::u64:
        case ElementType::f64: return 8;

        case ElementType::string: return sizeof(void *);
        }

        return 0;
    }

    inline void *Tensor::get_raw_data() const {
        return (void *)data.data();
    }

    inline void Tensor::set_raw_data(void *data) {
        this->data = {static_cast<const char *>(data), get_size_in_bytes()};
    }

    inline void Tensor::expect(const Tensor &actual) const {
        std::string_view name = this->name;
        bool             ok   = false;

        name = !name.empty() && name.back() == '?' ? name.substr(0, name.size() - 1) : name;
        for (size_t i; !name.empty(); name = name.substr(i + 1)) {
            i = name.find('|');
            if (actual.name == name.substr(0, i)) {
                ok = true;
                break;
            }
            if (i == std::string::npos) {
                break;
            }
        }

        if (!ok)
            throw std::runtime_error("name mismatch. Expect `" + this->name + "`, got `" + actual.name + "`");

        if (auto [a, b] = std::tie(this->element_type, actual.element_type); a != b)
            throw std::runtime_error(
                "element type mismatch. Expect `" + std::to_string((int)a) + "`, got `" + std::to_string((int)b) + "`"
            );

        if (auto [a, b] = std::make_pair(this->shape.size(), actual.shape.size()); a != b)
            throw std::runtime_error("shape size mismatch. Expect size=" + std::to_string(a) + ", got size=" + std::to_string(b));

        for (size_t i = 0; i < this->shape.size(); ++i) {
            auto &dim_a = this->shape[i];
            auto &dim_b = actual.shape[i];
            if (auto [pa, pb] =
                    std::make_pair(std::get_if<Tensor::StaticDimension>(&dim_a), std::get_if<Tensor::StaticDimension>(&dim_b));
                pa && pb && *pa != *pb)
                throw std::runtime_error(
                    "dimension mismatch at index=" + std::to_string(i) + ", expect " + std::to_string(*pa) + " got " +
                    std::to_string(*pb)
                );
            else if (
                auto [pa, pb] = std::make_pair(std::get_if<std::string>(&dim_a), std::get_if<std::string>(&dim_b));
                pa && pb && *pa != *pb
            )
                throw std::runtime_error("dimension mismatch at index=" + std::to_string(i) + ", expect " + *pa + " got " + *pb);
        }
    }

    inline void Tensor::expect_all(const std::vector<Tensor> &expected, const std::vector<Tensor> &actual) {
        if (actual.size() > expected.size()) {
            throw std::runtime_error(
                "Tensor count mismatch. Expected at most " + std::to_string(expected.size()) + ", got " +
                std::to_string(actual.size())
            );
        }

        for (size_t i = 0; i < actual.size(); ++i) {
            try {
                expected[i].expect(actual[i]);
            } catch (const std::exception &e) {
                throw std::runtime_error("Tensor[" + std::to_string(i) + "]: " + e.what());
            }
        }

        for (size_t i = actual.size(); i < expected.size(); ++i) {
            const auto &name = expected[i].name;
            if (name.empty() || name.back() != '?') {
                throw std::runtime_error("Missing required tensor `" + name + "`");
            }
        }
    }
} // namespace cpx::inference


template <>
struct cpx::Reflect<cpx::inference::Tensor::ElementType> {
    using Self = cpx::inference::Tensor::ElementType;

    static void to_str(const Self &self, std::string &str) {
        switch (self) {
        case Self::dynamic: str = "dynamic"; break;
        case Self::boolean: str = "boolean"; break;
        case Self::bf16: str = "bf16"; break;
        case Self::f16: str = "f16"; break;
        case Self::f32: str = "f32"; break;
        case Self::f64: str = "f64"; break;
        case Self::i4: str = "i4"; break;
        case Self::i8: str = "i8"; break;
        case Self::i16: str = "i16"; break;
        case Self::i32: str = "i32"; break;
        case Self::i64: str = "i64"; break;
        case Self::u1: str = "u1"; break;
        case Self::u2: str = "u2"; break;
        case Self::u3: str = "u3"; break;
        case Self::u4: str = "u4"; break;
        case Self::u6: str = "u6"; break;
        case Self::u8: str = "u8"; break;
        case Self::u16: str = "u16"; break;
        case Self::u32: str = "u32"; break;
        case Self::u64: str = "u64"; break;
        case Self::nf4: str = "nf4"; break;
        case Self::f8e4m3: str = "f8e4m3"; break;
        case Self::f8e5m2: str = "f8e5m2"; break;
        case Self::string: str = "string"; break;
        case Self::f4e2m1: str = "f4e2m1"; break;
        case Self::f8e8m0: str = "f8e8m0"; break;
        }
    }

    static void from_str(Self &self, std::string_view str) {
        if (str == "dynamic")
            self = Self::dynamic;
        else if (str == "boolean")
            self = Self::boolean;
        else if (str == "bf16")
            self = Self::bf16;
        else if (str == "f16")
            self = Self::f16;
        else if (str == "f32")
            self = Self::f32;
        else if (str == "f64")
            self = Self::f64;
        else if (str == "i4")
            self = Self::i4;
        else if (str == "i8")
            self = Self::i8;
        else if (str == "i16")
            self = Self::i16;
        else if (str == "i32")
            self = Self::i32;
        else if (str == "i64")
            self = Self::i64;
        else if (str == "u1")
            self = Self::u1;
        else if (str == "u2")
            self = Self::u2;
        else if (str == "u3")
            self = Self::u3;
        else if (str == "u4")
            self = Self::u4;
        else if (str == "u6")
            self = Self::u6;
        else if (str == "u8")
            self = Self::u8;
        else if (str == "u16")
            self = Self::u16;
        else if (str == "u32")
            self = Self::u32;
        else if (str == "u64")
            self = Self::u64;
        else if (str == "nf4")
            self = Self::nf4;
        else if (str == "f8e4m3")
            self = Self::f8e4m3;
        else if (str == "f8e5m2")
            self = Self::f8e5m2;
        else if (str == "string")
            self = Self::string;
        else if (str == "f4e2m1")
            self = Self::f4e2m1;
        else if (str == "f8e8m0")
            self = Self::f8e8m0;
        else
            throw std::invalid_argument("invalid Tensor::ElementType");
    }
};
#endif
