#include <gtest/gtest.h>
#include <tensile.hpp>

class DenseVectorTest : public testing::Test {
protected:

    DenseVector<double> vec{1.0, 2.0, 3.0, 4.0, 5.0};
    DenseVector<double> empty_vec;
};

TEST_F(DenseVectorTest, VectorInitializationWorks) {
    ASSERT_EQ(vec.size(), 5) << "vec.size() does not equal 5, instead is " << vec.size() << '\n' ;
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], static_cast<double>(i + 1));
    }
}

TEST_F(DenseVectorTest, ElementRetrievalandAssignmentNoBoundsChecking) {
    ASSERT_EQ(vec[0], 1.0) << "vec[0] does not equal 1, instead is " << vec[0] << '\n';
    vec[1] = 1.5;
    ASSERT_EQ(vec[1], 1.5) << "vec[1] does not equal 1,5, instead is " << vec[1] << '\n';
}

TEST_F(DenseVectorTest, ElementRetrievalandAssignmentBoundsChecking) {
    ASSERT_EQ(vec.at(0), 1.0);
    vec.at(1) = 1.5;
    ASSERT_EQ(vec.at(1), 1.5);
    ASSERT_EQ(vec.at(-1), 5.0);
}
TEST_F(DenseVectorTest, SelfAdditionWorks) {
    vec += 1.0;
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], i + 2.0);
    }
}

TEST_F(DenseVectorTest, SelfSubtractionWorks) {
    vec -= 1.0;
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], static_cast<double>(i));
    }
}

TEST_F(DenseVectorTest, SelfMultiplicationWorks) {
    vec *= 1.0;
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], static_cast<double>(i + 1));
    }
}

TEST_F(DenseVectorTest, SelfDivisionWorks) {
    vec /= 2.0;
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], static_cast<double>((i + 1.0) / 2.0));
    }
}