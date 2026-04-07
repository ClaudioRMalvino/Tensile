#include <gtest/gtest.h>
#include <tensile.hpp>

TEST(DenseVectorTest, VectorInitializationWorks) {
    DenseVector<double> vec{1.0, 2.0, 3.0, 4.0, 5.0};
    ASSERT_EQ(vec.size(), 5) << "vec.size() does not equal 5, instead is " << vec.size() << '\n' ;
}

TEST(DenseVectorTest, ElementRetrievalandAssignmentWorks) {
    DenseVector<double> vec{1.0, 2.0, 3.0, 4.0, 5.0};
    ASSERT_EQ(vec[0], 1) << "vec[0] does not equal 1, instead is " << vec[0] << '\n';
    vec[1] = 1.5;
    ASSERT_EQ(vec[1], 1.5) << "vec[1] does not equal 1,5, instead is " << vec[1] << '\n';
    ASSERT_EQ(vec[-1], 5.0) << "vec[-1] does not equal 5.0, instead is " << vec[-1] << '\n';
}

TEST(DenseVectorTest, SelfAdditionWorks) {
    DenseVector<double> vec{1.0, 2.0, 3.0, 4.0, 5.0};
    vec += 1.0;
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], i + 2.0);
    }
}

TEST(DenseVectorTest, SelfSubtractionWorks) {
    DenseVector<double> vec{1.0, 2.0, 3.0, 4.0, 5.0};
    vec -= 1.0;
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], static_cast<double>(i));
    }
}

TEST(DenseVectorTest, SelfMultiplicationWorks) {
    DenseVector<double> vec{1.0, 2.0, 3.0, 4.0, 5.0};
    vec *= 1.0;
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], static_cast<double>(i + 1));
    }
}

TEST(DenseVectorTest, SelfDivisionWorks) {
    DenseVector<double> vec{1.0, 2.0, 3.0, 4.0, 5.0};
    vec /= 2.0;
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], static_cast<double>((i + 1.0) / 2.0));
    }
}