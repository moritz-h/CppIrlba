#include <gtest/gtest.h>
#include "irlba/utils.hpp"
#include "Eigen/Dense"
#include "Eigen/Sparse"
#include "NormalSampler.h"

TEST(UtilsTest, Orthogonalize) {
    // Generated by svd(matrix(rnorm(20), 5, 4))$u
    Eigen::MatrixXd m(5, 4);
    m <<  0.1186600,  0.9038716,  0.10194199,  0.2484399218,
         -0.3090796, -0.1659502,  0.52915799, -0.3366910627,
         -0.3107643, -0.2636561, -0.35950860,  0.7248074228,
         -0.1683330,  0.1634341, -0.75523159, -0.5473305105,
          0.8749166, -0.2434161, -0.09989224, -0.0004957394;

    auto orthogonalize_vector = [](const Eigen::MatrixXd& mat, Eigen::VectorXd& vec, size_t ncols) -> void {
        Eigen::VectorXd tmp(mat.cols());
        irlba::orthogonalize_vector(mat, vec, ncols, tmp);
    };

    // Generated by rnorm(5)
    Eigen::VectorXd v(5);
    v << -0.24054848, -0.04785069, -0.76491749, -0.65634291, 0.62815141;

    auto copy = v;
    orthogonalize_vector(m, copy, 4);

    // Checking that the L2 norm is not all-zero.
    double l2 = 0;
    for (auto x : copy) { l2 += x*x; }
    EXPECT_TRUE(l2 > 0.1);

    // Checking that we do have orthogonality.
    for (size_t i = 0; i < 4; ++i) {
        auto col = m.col(i);
        auto cit = col.begin();
        double sum = 0;
        for (auto x : copy) {
            sum += x * (*cit);
        }
        EXPECT_TRUE(sum < 0.0000000001);
    }

    // Checking that the specification of columns has an effect.
    auto copy2 = v;
    orthogonalize_vector(m, copy, 2);
    EXPECT_NE(copy, copy2);
}

TEST(UtilsTest, FillNormals) {
    Eigen::VectorXd test(10);
    test.setZero();
    std::mt19937_64 eng(1000);

    // Filled with non-zeros.
    irlba::fill_with_random_normals(test, eng);
    for (auto v : test) {
        EXPECT_NE(v, 0);
    }
    
    std::sort(test.begin(), test.end());
    for (size_t v = 1; v < test.size(); ++v) {
        EXPECT_NE(test[v], test[v-1]);
    }

    // Works for odd sizes.
    Eigen::VectorXd test2(11);
    test2.setZero();

    irlba::fill_with_random_normals(test2, eng);
    for (auto v : test2) {
        EXPECT_NE(v, 0);
    }

    std::sort(test2.begin(), test2.end());
    for (size_t v = 1; v < test2.size(); ++v) {
        EXPECT_NE(test2[v], test2[v-1]);
    }

    // Works for matrices.
    Eigen::MatrixXd test3(13, 2);
    test3.setZero();

    irlba::fill_with_random_normals(test3, 0, eng);
    auto first = test3.col(0);
    for (auto v : first) {
        EXPECT_NE(v, 0);
    }

    auto second = test3.col(1);
    for (auto v : second) {
        EXPECT_EQ(v, 0);
    }
}

struct dummy_class {
    Eigen::MatrixXd realize() { return Eigen::MatrixXd(); }
};

TEST(UtilsTest, ConvertibleCheck) {
    EXPECT_FALSE(irlba::has_realize_method<Eigen::MatrixXd>::value);
    EXPECT_FALSE(irlba::has_realize_method<Eigen::SparseMatrix<double> >::value);
    EXPECT_TRUE(irlba::has_realize_method<dummy_class>::value);

    EXPECT_TRUE(irlba::has_multiply_method<Eigen::MatrixXd>::value);
    EXPECT_TRUE(irlba::has_adjoint_multiply_method<Eigen::MatrixXd>::value);
    EXPECT_TRUE(irlba::has_multiply_method<Eigen::SparseMatrix<double> >::value);
    EXPECT_TRUE(irlba::has_adjoint_multiply_method<Eigen::SparseMatrix<double> >::value);
}

TEST(UtilsTest, NormalSampler) {
    NormalSampler norm(10);

    // Different results.
    double first = norm();
    double second = norm();
    EXPECT_NE(first, second);

    // Same results.
    NormalSampler norm2(10);
    double first2 = norm2();
    EXPECT_EQ(first, first2);
}

