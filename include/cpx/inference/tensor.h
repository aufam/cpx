#ifndef CPX_INFERENCE_TENSOR_H
#define CPX_INFERENCE_TENSOR_H

#include <cpx/tag.h>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <variant>

namespace cpx::inference {
    struct Tensor {
        enum class ElementType {
            dynamic, //!< Dynamic element type
            boolean, //!< boolean element type
            bf16,    //!< bf16 element type
            f16,     //!< f16 element type
            f32,     //!< f32 element type
            f64,     //!< f64 element type
            i4,      //!< i4 element type
            i8,      //!< i8 element type
            i16,     //!< i16 element type
            i32,     //!< i32 element type
            i64,     //!< i64 element type
            u1,      //!< binary element type
            u2,      //!< u2 element type
            u3,      //!< u3 element type
            u4,      //!< u4 element type
            u6,      //!< u6 element type
            u8,      //!< u8 element type
            u16,     //!< u16 element type
            u32,     //!< u32 element type
            u64,     //!< u64 element type
            nf4,     //!< nf4 element type
            f8e4m3,  //!< f8e4m3 element type
            f8e5m2,  //!< f8e5m2 element type
            string,  //!< string element type
            f4e2m1,  //!< f4e2m1 element type
            f8e8m0,  //!< f8e8m0 element type
        };

        using StaticDimension  = size_t;
        using DynamicDimension = std::variant<StaticDimension, std::string>;

        using StaticShape  = std::vector<StaticDimension>;
        using DynamicShape = std::vector<DynamicDimension>;

        Tag<std::string>  name         = {"json,toml:`name`"};
        Tag<DynamicShape> shape        = {"json,toml:`shape`"};
        Tag<ElementType>  element_type = {"json:`elementType` toml:`element-type`"};

        template <typename T = StaticDimension, typename = std::enable_if_t<std::is_convertible_v<T, StaticDimension>>>
        void set_shape(const std::vector<T> &shape) {
            this->shape().resize(shape.size(), 0ul);
            for (size_t i = 0; i < shape.size(); ++i)
                this->shape()[i] = StaticDimension(shape[i]);
        }

        template <typename T = StaticDimension, typename = std::enable_if_t<std::is_convertible_v<T, StaticDimension>>>
        std::vector<T> get_shape() const {
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

            auto &self_shape = this->shape();
            auto  shape      = std::vector<T>(self_shape.size());
            auto  visitor    = ToStaticVisitor{self_shape};
            for (size_t i = 0; i < self_shape.size(); ++i)
                shape[i] = T(std::visit(visitor, self_shape[i]));

            return shape;
        }

        void *_data = nullptr;

        void *get_raw_data() const {
            return _data;
        }

        void set_raw_data(void *data) {
            _data = data;
        }

        template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        T *get_data() const {
            if (sizeof(T) != get_element_size())
                throw std::runtime_error("Mismatch element size. Forgot to define element type?");
            return static_cast<T *>(_data);
        }

        template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        std::vector<T> get_data_as_vector() const {
            auto begin = get_data<T>();
            auto end   = begin + get_size();
            return {begin, end};
        }

        template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        void set_data(const std::vector<T> &data) {
            if (sizeof(T) != get_element_size())
                throw std::runtime_error("Mismatch element size. Forgot to define element type?");
            if (data.size() != get_size())
                throw std::runtime_error("Mismatch shape. Forgot to define static shape?");
            _data = (void *)data.data();
        }

        size_t get_size() const {
            size_t res = 1;
            for (auto dim : get_shape())
                res *= dim;
            return res;
        }

        size_t get_size_in_bytes() const {
            if (auto elem_size = get_element_size(); elem_size != 0)
                return get_size() * elem_size;
            throw std::runtime_error("Element type cannot be dynamic");
        }

        size_t get_element_size() const {
            switch (element_type()) {
            case ElementType::dynamic:
                return 0;

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
            case ElementType::f4e2m1:
                return 1;

            case ElementType::i16:
            case ElementType::u16:
            case ElementType::bf16:
            case ElementType::f16:
                return 2;

            case ElementType::i32:
            case ElementType::u32:
            case ElementType::f32:
                return 4;

            case ElementType::i64:
            case ElementType::u64:
            case ElementType::f64:
                return 8;

            case ElementType::string:
                return sizeof(void *);
            }

            return 0;
        }
    };

    struct ProtoTensor {
        Tag<std::string>                    name         = "protobuf:`1`";
        Tag<std::vector<size_t>>            shape        = "protobuf:`2`";
        Tag<inference::Tensor::ElementType> element_type = "protobuf:`3`";
        Tag<std::vector<uint8_t>>           data         = "protobuf:`4`";

        static ProtoTensor from(const inference::Tensor &tensor) {
            ProtoTensor t;
            t.name() = tensor.name();
            for (auto &dim : tensor.shape()) {
                if (auto d = std::get_if<size_t>(&dim)) {
                    t.shape().push_back(*d);
                } else {
                    t.shape().push_back(0);
                }
            }
            t.element_type() = tensor.element_type();

            if (auto begin = static_cast<uint8_t *>(tensor.get_raw_data())) {
                auto end = begin + tensor.get_size_in_bytes();
                t.data() = std::vector<uint8_t>(begin, end);
            }
            return t;
        }

        inference::Tensor into() const {
            inference::Tensor t;
            t.name() = name();
            for (auto &dim : shape()) {
                if (dim)
                    t.shape().emplace_back(dim);
                else
                    t.shape().emplace_back("?");
            }
            t.element_type() = element_type();
            if (data().size())
                t.set_raw_data((void *)data().data());

            return t;
        }
    };

    struct ProtoTensorIOs {
        Tag<std::vector<ProtoTensor>> inputs  = "protobuf:`1`";
        Tag<std::vector<ProtoTensor>> outputs = "protobuf:`2`";
    };

    static_assert(std::is_aggregate_v<Tensor>, "Tensor must be an aggregate");
    static_assert(std::is_aggregate_v<ProtoTensor>, "Tensor must be an aggregate");
    static_assert(std::is_aggregate_v<ProtoTensorIOs>, "Tensor must be an aggregate");
} // namespace cpx::inference

#endif
