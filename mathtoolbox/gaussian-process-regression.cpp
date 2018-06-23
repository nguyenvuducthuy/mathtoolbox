#include "gaussian-process-regression.hpp"
#include <Eigen/LU>

using Eigen::VectorXd;
using Eigen::MatrixXd;

namespace
{
    // Equation 5.1
    double CalculateArdSquaredExponentialKernel(const VectorXd& x_i, const VectorXd& x_j, const double s_f_squared, const VectorXd& l)
    {
        const int    D   = x_i.rows();
        const double sum = [&]()
        {
            double sum = 0.0;
            for (int i = 0; i < D; ++ i)
            {
                sum += (x_i(i) - x_j(i)) * (x_i(i) - x_j(i)) / (l(i) * l(i));
            }
            return sum;
        }();
        
        return s_f_squared * std::exp(- 0.5 * sum);
    }
    
    MatrixXd CalculateLargeK(const MatrixXd& X, const double s_f_squared, const double s_n_squared, const VectorXd& l)
    {
        const int      N = X.cols();
        const MatrixXd K = [&]()
        {
            MatrixXd K(N, N);
            for (unsigned i = 0; i < N; ++ i)
            {
                for (unsigned j = i; j < N; ++ j)
                {
                    const double value = CalculateArdSquaredExponentialKernel(X.col(i), X.col(j), s_f_squared, l);
                    K(i, j) = value;
                    K(j, i) = value;
                }
            }
            return K;
        }();
        
        return K + s_n_squared * MatrixXd::Identity(N, N);
    }
    
    VectorXd CalculateSmallK(const VectorXd& x, const MatrixXd& X, const double s_f_squared, const VectorXd& l)
    {
        const int      N = X.cols();
        const VectorXd k = [&]()
        {
            VectorXd k(N);
            for (unsigned i = 0; i < N; ++ i)
            {
                k(i) = CalculateArdSquaredExponentialKernel(x, X.col(i), s_f_squared, l);
            }
            return k;
        }();
        
        return k;
    }
    
    // Equation 5.8
    double CalculateLogLikelihood(const MatrixXd& X, const VectorXd& y, const double s_f_squared, const double s_n_squared, const VectorXd& l)
    {
        const int N = X.cols();

        const MatrixXd K     = CalculateLargeK(X, s_f_squared, s_n_squared, l);
        const MatrixXd K_inv = K.inverse();

        const double term1 = - 0.5 * y.transpose() * K_inv * y;
        const double term2 = - 0.5 * std::log(K.determinant());
        const double term3 = - 0.5 * N * std::log(2.0 * M_PI);

        return term1 + term2 + term3;
    }
}

namespace mathtoolbox
{
    GaussianProcessRegression::GaussianProcessRegression(const MatrixXd& X, const VectorXd& y) : X(X), y(y)
    {
        const int D = X.rows();
        
        SetHyperparameters(0.10, 1e-05, VectorXd::Constant(D, 0.10));
    }
    
    void GaussianProcessRegression::SetHyperparameters(double s_f_squared, double s_n_squared, const Eigen::VectorXd& l)
    {
        this->s_f_squared = s_f_squared;
        this->s_n_squared = s_n_squared;
        this->l           = l;

        K     = CalculateLargeK(X, s_f_squared, s_n_squared, l);
        K_inv = K.inverse();
    }
    
    void GaussianProcessRegression::PerformMaximumLikelihood(double s_f_squared, double s_n_squared, const Eigen::VectorXd& l)
    {
        // TODO

        K     = CalculateLargeK(X, s_f_squared, s_n_squared, l);
        K_inv = K.inverse();
    }

    double GaussianProcessRegression::EstimateY(const VectorXd& x) const
    {
        const VectorXd k = CalculateSmallK(x, X, s_f_squared, l);
        return k.transpose() * K_inv * y;
    }
    
    double GaussianProcessRegression::EstimateS(const VectorXd& x) const
    {
        const VectorXd k = CalculateSmallK(x, X, s_f_squared, l);
        return std::sqrt(s_f_squared + s_n_squared - k.transpose() * K_inv * k);
    }
}
