#include <gtest/gtest.h>
#include <tensile.hpp>
#include <vector>
#include <iostream>

class DenseVectorTest : public testing::Test {
protected:

    DenseVector<double> vec{1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<double> vec2{1.0, 2.0, 3.0, 4.0, 5.0};
    DenseVector<double> vec3{1.0, 2.0, 3.0, 4.0, 5.0};
    DenseVector<double> empty_vec;

};

TEST_F(DenseVectorTest, InitializationAfterInstantiationWorks) {
    empty_vec = {1.0, 2.0, 3.0, 4.0, 5.0};
    ASSERT_EQ(empty_vec.size(), vec.size());
    ASSERT_EQ(empty_vec[0], vec[0]);
}

TEST_F(DenseVectorTest, VectorInitializationWorks) {
    ASSERT_EQ(vec.size(), 5) << "vec.size() does not equal 5, instead is " << vec.size() << '\n' ;
    for (size_t i{0}; i < vec.size(); ++i) {
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

TEST_F(DenseVectorTest, PowFunctionWorks) {
    DenseVector<double> expected {1.0, 4.0, 9.0, 16.0, 25.0};
    DenseVector<double> result = vec.pow(2);
    vec.pow(1);
    for (size_t i{0}; i < vec.size(); ++i) {
        std::cout << "result = " << result[i] << '\n';
        std::cout << "vec = " << vec[i] << '\n';
    }
    for (size_t i{0}; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], vec[i]);
        ASSERT_EQ(result[i], expected[i]);
    }

}

TEST_F(DenseVectorTest, ScalarAdditionWorks) {
    vec += 1.0;
    for (size_t i{0}; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], i + 2.0);
    }
    DenseVector<double> result = vec + 1.0;
    for (size_t i{0}; i < result.size(); ++i) {
        ASSERT_EQ(result[i], i + 3.0);
    }
}

TEST_F(DenseVectorTest, ScalarSubtractionWorks) {
    vec -= 1.0;
    for (size_t i{0}; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], static_cast<double>(i));
    }
    DenseVector<double> result = vec - 1.0;
    for (size_t i{0}; i < vec.size(); ++i) {
        ASSERT_EQ(result[i], static_cast<double>(i) - 1.0);
    }
}

TEST_F(DenseVectorTest, ScalarMultiplicationWorks) {
    vec *= 1.0;
    for (size_t i{0}; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], static_cast<double>(i + 1));
    }
    DenseVector<double> result = vec * 1.0;
    for (size_t i{0}; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], static_cast<double>(i + 1));
    }
}

TEST_F(DenseVectorTest, ScalarDivisionWorks) {
    vec /= 2.0;
    for (size_t i{0}; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], static_cast<double>((i + 1.0) / 2.0));
    }
    DenseVector<double> result = vec / 2.0;
    for (size_t i{0}; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], static_cast<double>((i + 1.0) / 2.0));
    }
}

TEST_F(DenseVectorTest, VectorAdditionWorks) {
    DenseVector<double> expected {2.0, 4.0, 6.0, 8.0, 10.0};
    DenseVector<double> result = vec + vec2;
    DenseVector<double> result2 = vec + vec3;
    vec += vec2;
    for (size_t i{0}; i < vec.size(); ++i) {
      ASSERT_EQ(result[i], expected[i]);
        ASSERT_EQ(result2[i], expected[i]);
        ASSERT_EQ(vec[i], expected[i]);
    }
}

TEST_F(DenseVectorTest, VectorSubtractionWorks) {
    DenseVector<double> expected {0.0, 0.0, 0.0, 0.0, 0.0};
    DenseVector<double> result = vec - vec2;
    DenseVector<double> result2 = vec - vec3;
    vec -= vec2;
    for (size_t i{0}; i < vec.size(); ++i) {
        ASSERT_EQ(result[i], expected[i]);
        ASSERT_EQ(result2[i], expected[i]);
        ASSERT_EQ(vec[i], expected[i]);
    }
}

TEST_F(DenseVectorTest, VectorMultiplicationWorks) {
    DenseVector<double> expected {1.0, 4.0, 9.0, 16.0, 25.0};
    DenseVector<double> result = vec * vec2;
    DenseVector<double> result2 = vec * vec3;
    vec *= vec2;
    for (size_t i{0}; i < vec.size(); ++i) {
        std::cout << "result = " << result[i] << '\n';
        std::cout << "result2 = " << result2[i] << '\n';
        std::cout << "self vec = " << vec[i] << '\n';
    }
    for (size_t i{0}; i < vec.size(); ++i) {
        ASSERT_EQ(result[i], expected[i]);
        ASSERT_EQ(result2[i], expected[i]);
        ASSERT_EQ(vec[i], expected[i]);
    }
}