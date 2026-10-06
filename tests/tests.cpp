#include <gtest/gtest.h>
#include <tensile.hpp>
#include <vector>

using tensile::DenseMatrix;
using tensile::DenseVector;

class DenseMatrixTest : public testing::Test {
protected:
    DenseMatrix<int> matrix{{1,2,3,4}, {5,6,7,8}};
    const DenseMatrix<int> const_matrix{{1,2,3,4}, {5,6,7,8}};
    DenseMatrix<double> empty_mat;
};

class DenseVectorTest : public testing::Test {
   protected:
    DenseVector<double> vec{1.0, 2.0, 3.0, 4.0, 5.0};
    const DenseVector<double> const_vec{1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<double> stl_vec{1.0, 2.0, 3.0, 4.0, 5.0};
    DenseVector<double> vec3{1.0, 2.0, 3.0, 4.0, 5.0};
    DenseVector<double> empty_vec;
};

TEST_F(DenseMatrixTest, DenseMatrixLayoutWorks) {
    DenseMatrix<double> row_major_matrix;
    DenseMatrix<double, tensile::ColMajor> col_major_matrix;

    EXPECT_TRUE((std::is_same_v<decltype(row_major_matrix.layout()), tensile::RowMajor>));
    EXPECT_TRUE((std::is_same_v<decltype(col_major_matrix.layout()), tensile::ColMajor>));
}

TEST_F(DenseMatrixTest, ListInitializationWorks) {
    std::vector<std::vector<int>> stl_matrix{{1, 2, 3, 4}, {5, 6, 7, 8}};

    for (size_t idx_i{}; idx_i < matrix.rows(); ++idx_i)
    {
        for (size_t idx_j{}; idx_j < matrix.cols(); ++idx_j)
        {
            ASSERT_EQ((stl_matrix[idx_i][idx_j]), (matrix[idx_i,idx_j]));
        }
    }
}

TEST_F(DenseMatrixTest, SquareConstructorWorks) {
    DenseMatrix<double> sq_matrix(4);
    size_t expected_size{16};

    ASSERT_EQ(sq_matrix.size(), expected_size);
}

TEST_F(DenseMatrixTest, RectangularConstructionWorks) {
    DenseMatrix<double> rect_matrix(2,3);
    size_t expected_size{6};

    ASSERT_EQ(rect_matrix.size(), expected_size);
}

TEST_F(DenseMatrixTest, SquareValueInitializedWorks) {
    DenseMatrix<int> expected{{1,1,1},{1,1,1},{1,1,1}};
    DenseMatrix<int> mat(3, 3, 1);

    for (size_t idx_i{}; idx_i < mat.rows(); ++idx_i)
    {
        for (size_t idx_j{}; idx_j < mat.cols(); ++idx_j)
        {
            ASSERT_EQ((expected[idx_i, idx_j]), (mat[idx_i, idx_j]));
        }
    }
}

TEST_F(DenseMatrixTest, InitializationAfterInstantiationWorks) {
    empty_mat = {{1.0, 2.0, 3.0, 4.0}, {5.0, 6.0, 7.0, 8.0}};
    ASSERT_EQ(empty_mat.size(), matrix.size());

    for (size_t idx_i{}; idx_i < empty_mat.rows(); ++idx_i)
    {
        for (size_t idx_j{}; idx_j < empty_mat.cols(); ++idx_j)
        {
            ASSERT_EQ((empty_mat[idx_i, idx_j]), (matrix[idx_i, idx_j]));
        }
    }
}

TEST_F(DenseMatrixTest, ZeroInitializationAndZeroResetWorks) {
    DenseMatrix<int> expected {{0, 0, 0, 0}, {0, 0, 0, 0}};
    matrix.set_to_zeros();
    DenseMatrix init_to_zero = DenseMatrix<int>::zeros(2,4);

   for (size_t idx_i{}; idx_i < matrix.rows(); ++idx_i)
       {
           for (size_t idx_j{}; idx_j < matrix.cols(); ++idx_j)
           {
               ASSERT_EQ((expected[idx_i, idx_j]), (matrix[idx_i, idx_j]));
               ASSERT_EQ((expected[idx_i, idx_j]), (init_to_zero[idx_i, idx_j]));
           }
       }
}

TEST_F(DenseMatrixTest, InitializeWithIdentity) {
    DenseMatrix<double> expected{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}};
    DenseMatrix<double> identity_mat = DenseMatrix<double>::identity(3);

    for (size_t idx_i{}; idx_i < expected.rows(); ++idx_i)
    {
        for (size_t idx_j{}; idx_j < expected.cols(); ++idx_j)
        {
            ASSERT_EQ((expected[idx_i, idx_j]), (identity_mat[idx_i, idx_j]));
        }
    }
}

TEST_F(DenseMatrixTest, InitializedWithOnes) {
    DenseMatrix<int> expected{{1,1,1},{1,1,1},{1,1,1}};
    DenseMatrix ones_mat = DenseMatrix<int>::ones(3,3);

    for (size_t idx_i{}; idx_i < expected.rows(); ++idx_i)
        {
            for (size_t idx_j{}; idx_j < expected.cols(); ++idx_j)
            {
                ASSERT_EQ((expected[idx_i, idx_j]), (ones_mat[idx_i, idx_j]));
            }
        }
}

TEST_F(DenseVectorTest, InitializationAfterInstantiationWorks) {
    empty_vec = {1.0, 2.0, 3.0, 4.0, 5.0};
    ASSERT_EQ(empty_vec.size(), vec.size());

    for (size_t idx{}; idx < vec.size(); ++idx){
        ASSERT_EQ(empty_vec[idx], vec[idx]);
    }
}

TEST_F(DenseVectorTest, InitializationWorks) {
    ASSERT_EQ(vec.size(), 5) << "vec.size() does not equal 5, instead is " << vec.size() << '\n';
    for (size_t i{0}; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], static_cast<double>(i + 1));
    }
}

TEST_F(DenseVectorTest, InitializedWithValueWorks) {
    DenseVector<double> expected{1.0, 1.0, 1.0, 1.0};
    DenseVector<double> vect(4, 1.0);

    for (size_t idx{}; idx < vect.size(); ++idx)
    {
        ASSERT_EQ((expected[idx]), (vect[idx]));
    }
}

TEST_F(DenseVectorTest, ZeroInitializationAndZeroResetWorks) {
    DenseVector<double> expected{0.0, 0.0, 0.0, 0.0, 0.0};
    vec3.set_to_zeros();
    DenseVector vec_initialized_zero = DenseVector<double>::zeros(5);

    for (size_t i{0}; i < expected.size(); ++i) {
        ASSERT_EQ(expected[i], vec_initialized_zero[i]);
        ASSERT_EQ(expected[i], vec3[i]);
    }
}

TEST_F(DenseVectorTest, ElementRetrievalandAssignmentNoBoundsChecking) {
    ASSERT_EQ(vec[0], 1.0) << "vec[0] does not equal 1, instead is " << vec[0] << '\n';
    vec[1] = 1.5;
    ASSERT_EQ(vec[1], 1.5) << "vec[1] does not equal 1.5, instead is " << vec[1] << '\n';
}

TEST_F(DenseVectorTest, ElementRetrievalandAssignmentBoundsChecking) {
    // Non-const DenseVector check
    ASSERT_EQ(vec.at(0), 1.0);
    vec.at(1) = 1.5;
    ASSERT_EQ(vec.at(1), 1.5);
    ASSERT_EQ(vec.at(-1), 5.0);
    ASSERT_EQ(vec.at(-static_cast<int>(vec.size())), vec[0]);
    EXPECT_THROW(auto result = vec.at(static_cast<int>(vec.size())), std::out_of_range);
    EXPECT_THROW(auto result = vec.at(-static_cast<int>(vec.size()) - 1 ), std::out_of_range);

    //const DenseVector check
    ASSERT_EQ(const_vec.at(0), 1.0);
    ASSERT_EQ(const_vec.at(-1), 5.0);
    ASSERT_EQ(const_vec.at(-static_cast<int>(const_vec.size())), const_vec[0]);
    EXPECT_THROW(const auto result = const_vec.at(static_cast<int>(vec.size())), std::out_of_range);
    EXPECT_THROW(const auto result = const_vec.at(-static_cast<int>(vec.size()) - 1 ), std::out_of_range);
}

TEST_F(DenseVectorTest, PowFunctionWorks) {
    DenseVector<double> expected{1.0, 4.0, 9.0, 16.0, 25.0};
    DenseVector<double> expected2{1.0, 1.0, 1.0, 1.0, 1.0};

    DenseVector<double> result = pow(vec, 1);
    DenseVector<double> result2 = pow(vec, 0);
    vec.pow(2);

    for (size_t i{0}; i < vec.size(); ++i) {
        ASSERT_EQ(result[i], stl_vec[i]);
        ASSERT_EQ(vec[i], expected[i]);
        ASSERT_EQ(result2[i], expected2[i]);
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
    DenseVector<double> expected{2.0, 4.0, 6.0, 8.0, 10.0};
    DenseVector<double> result = vec + stl_vec;
    DenseVector<double> result2 = vec + vec3;
    vec += stl_vec;
    for (size_t i{0}; i < vec.size(); ++i) {
        ASSERT_EQ(result[i], expected[i]);
        ASSERT_EQ(result2[i], expected[i]);
        ASSERT_EQ(vec[i], expected[i]);
    }
}

TEST_F(DenseVectorTest, VectorSubtractionWorks) {
    DenseVector<double> expected{0.0, 0.0, 0.0, 0.0, 0.0};
    DenseVector<double> result = vec - stl_vec;
    DenseVector<double> result2 = vec - vec3;
    vec -= stl_vec;
    for (size_t i{0}; i < vec.size(); ++i) {
        ASSERT_EQ(result[i], expected[i]);
        ASSERT_EQ(result2[i], expected[i]);
        ASSERT_EQ(vec[i], expected[i]);
    }
}

TEST_F(DenseVectorTest, VectorMultiplicationWorks) {
    DenseVector<double> expected{1.0, 4.0, 9.0, 16.0, 25.0};
    DenseVector<double> result = vec * stl_vec;
    DenseVector<double> result2 = vec * vec3;
    vec *= stl_vec;

    for (size_t i{0}; i < vec.size(); ++i) {
        ASSERT_EQ(result[i], expected[i]);
        ASSERT_EQ(result2[i], expected[i]);
        ASSERT_EQ(vec[i], expected[i]);
    }
}

TEST_F(DenseVectorTest, VectorDivisionWorks) {
    DenseVector<double> expected{1.0, 1.0, 1.0, 1.0, 1.0};
    DenseVector<double> result = vec / stl_vec;
    DenseVector<double> result2 = vec / vec3;
    vec /= stl_vec;

    for (size_t i{0}; i < vec.size(); ++i) {
        ASSERT_EQ(result[i], expected[i]);
        ASSERT_EQ(result2[i], expected[i]);
        ASSERT_EQ(vec[i], expected[i]);
    }
}
