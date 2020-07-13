/** Copyright 2017-2020 CNRS-AIST JRL and CNRS-UM LIRMM */

#include <tvm/manifold/DirectProduct.h>
#include <tvm/manifold/Real.h>
#include <tvm/manifold/SO3.h>
#include <tvm/manifold/S3.h>
#include <tvm/manifold/internal/Adjoint.h>

#include <Eigen/Geometry>
#include <Eigen/LU>                           // for inverse
#include <unsupported/Eigen/MatrixFunctions>  // for generic log and exp on matrices

#include <iostream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include "doctest/doctest.h"

using namespace tvm;
using namespace Eigen;

TEST_CASE("sincsqrt")
{
  double x = 1.;
  for (int i = 0; i < 18; ++i)
  {
    FAST_CHECK_EQ(tvm::manifold::internal::sinc(x), tvm::manifold::internal::sincsqrt(x * x));
    x /= 10;
  }
}

TEST_CASE("R3")
{
  using R3 = manifold::Real<3>;
  Vector3d x = Vector3d::Random();
  Vector3d y = Vector3d::Random();
  MatrixXd M = MatrixXd::Random(3, 4);
  auto z = M.row(0).transpose().head(3);
  FAST_CHECK_EQ(R3::dim, 3);
  FAST_CHECK_EQ(R3::dynamicDim(x), 3);
  FAST_CHECK_UNARY((x + y).isApprox(R3::compose(x, y)));
  FAST_CHECK_UNARY(x.isApprox(R3::compose(x, R3::identity)));
  FAST_CHECK_UNARY(y.isApprox(R3::compose(R3::identity, y)));
  FAST_CHECK_UNARY((z + x + y).isApprox(R3::compose(z, x+y)));
  FAST_CHECK_UNARY((-x).isApprox(R3::inverse(x)));
  FAST_CHECK_UNARY(std::is_same_v<decltype(R3::inverse(R3::identity)),R3::Identity>);

  auto l = R3::log(x);
  FAST_CHECK_UNARY(x.isApprox(l));
  FAST_CHECK_UNARY(std::is_same_v<decltype(R3::log(R3::identity)), R3::AlgIdentity>);
  FAST_CHECK_UNARY(x.isApprox(R3::exp(l)));
  FAST_CHECK_UNARY(std::is_same_v<decltype(R3::exp(R3::algIdentity)), R3::Identity>);
  FAST_CHECK_UNARY((x - y).isApprox(R3::compose(R3::exp(l), R3::inverse(y))));
}

TEST_CASE("Rn")
{
  using Rn = manifold::Real<Dynamic>;
  VectorXd x = VectorXd::Random(8);
  VectorXd y = VectorXd::Random(8);
  MatrixXd M = MatrixXd::Random(3, 10);
  auto z = M.row(0).transpose().head(8);
  FAST_CHECK_EQ(Rn::dim, Dynamic);
  FAST_CHECK_EQ(Rn::dynamicDim(x), 8);
  FAST_CHECK_UNARY((x + y).isApprox(Rn::compose(x, y)));
  FAST_CHECK_UNARY(x.isApprox(Rn::compose(x, Rn::identity)));
  FAST_CHECK_UNARY(y.isApprox(Rn::compose(Rn::identity, y)));
  FAST_CHECK_UNARY((z + x + y).isApprox(Rn::compose(z, x + y)));
  FAST_CHECK_UNARY((-x).isApprox(Rn::inverse(x)));
  FAST_CHECK_UNARY(std::is_same_v<decltype(Rn::inverse(Rn::identity)), Rn::Identity>);

  auto l = Rn::log(x);
  FAST_CHECK_UNARY(x.isApprox(l));
  FAST_CHECK_UNARY(std::is_same_v<decltype(Rn::log(Rn::identity)), Rn::AlgIdentity>);
  FAST_CHECK_UNARY(x.isApprox(Rn::exp(l)));
  FAST_CHECK_UNARY(std::is_same_v<decltype(Rn::exp(Rn::algIdentity)), Rn::Identity>);
  FAST_CHECK_UNARY((x - y).isApprox(Rn::compose(Rn::exp(l), Rn::inverse(y))));
}

TEST_CASE("SO3")
{
  using SO3 = manifold::SO3;
  Matrix3d x = Quaterniond::UnitRandom().toRotationMatrix();
  Matrix3d y = Quaterniond::UnitRandom().toRotationMatrix();
  MatrixXd M = MatrixXd::Random(3, 4);
  FAST_CHECK_EQ(SO3::dim, 3);
  FAST_CHECK_EQ(SO3::dynamicDim(x), 3);
  auto z = M.leftCols<3>();
  FAST_CHECK_UNARY((x * y).isApprox(SO3::compose(x, y)));
  FAST_CHECK_UNARY(x.isApprox(SO3::compose(x, SO3::identity)));
  FAST_CHECK_UNARY(y.isApprox(SO3::compose(SO3::identity, y)));
  FAST_CHECK_UNARY((z * x * y).isApprox(SO3::compose(z, x * y)));
  FAST_CHECK_UNARY(x.transpose().isApprox(SO3::inverse(x)));
  FAST_CHECK_UNARY(std::is_same_v<decltype(SO3::inverse(SO3::identity)), SO3::Identity>);

  auto l = SO3::log(x);
  FAST_CHECK_UNARY(SO3::vee(SO3::hat(l)).isApprox(l));
  FAST_CHECK_UNARY(Matrix3d(x.log()).isApprox(SO3::hat(l)));
  FAST_CHECK_UNARY(std::is_same_v<decltype(SO3::log(SO3::identity)), SO3::AlgIdentity>);
  FAST_CHECK_UNARY(x.isApprox(SO3::exp(l)));
  FAST_CHECK_UNARY(std::is_same_v<decltype(SO3::exp(SO3::algIdentity)), SO3::Identity>);
  FAST_CHECK_UNARY((x * y.transpose()).isApprox(SO3::compose(SO3::exp(l), SO3::inverse(y))));
}

TEST_CASE("S3")
{
  using S3 = manifold::S3;
  Quaterniond x = Quaterniond::UnitRandom();
  Quaterniond y = Quaterniond::UnitRandom();
  Quaterniond z = Quaterniond::UnitRandom();
  FAST_CHECK_EQ(S3::dim, 3);
  FAST_CHECK_EQ(S3::dynamicDim(x), 3);
  FAST_CHECK_UNARY((x * y).isApprox(S3::compose(x, y)));
  FAST_CHECK_UNARY(x.isApprox(S3::compose(x, S3::identity)));
  FAST_CHECK_UNARY(y.isApprox(S3::compose(S3::identity, y)));
  FAST_CHECK_UNARY((z * x * y).isApprox(S3::compose(z, x * y)));
  FAST_CHECK_UNARY(x.inverse().isApprox(S3::inverse(x)));
  FAST_CHECK_UNARY(std::is_same_v<decltype(S3::inverse(S3::identity)), S3::Identity>);

  auto l = S3::log(x);
  FAST_CHECK_UNARY(S3::vee(S3::hat(l)).isApprox(l));
  FAST_CHECK_UNARY(x.toRotationMatrix().log().isApprox(S3::hat(l)));
  FAST_CHECK_UNARY(std::is_same_v<decltype(S3::log(S3::identity)), S3::AlgIdentity>);
  FAST_CHECK_UNARY(x.toRotationMatrix().isApprox(S3::exp(l).toRotationMatrix()));
  FAST_CHECK_UNARY(std::is_same_v<decltype(S3::exp(S3::algIdentity)), S3::Identity>);
  FAST_CHECK_UNARY((x * y.inverse()).toRotationMatrix().isApprox(S3::compose(S3::exp(l), S3::inverse(y)).toRotationMatrix()));
}

TEST_CASE("DirectProductUtils")
{
  using R3 = manifold::Real<3>;
  using R4 = manifold::Real<4>;
  using Rn = manifold::Real<Eigen::Dynamic>;
  using manifold::internal::product_traits;
  VectorXd x(7);
  VectorXd y(11);

  FAST_CHECK_EQ(product_traits<R3, R4>::elem1(x).size(), 3);
  FAST_CHECK_EQ(product_traits<R3, R4>::elem2(x).size(), 4);
  FAST_CHECK_EQ(product_traits<R3, Rn>::elem1(y).size(), 3);
  FAST_CHECK_EQ(product_traits<R3, Rn>::elem2(y).size(), 8);
  FAST_CHECK_EQ(product_traits<Rn, R3>::elem1(y).size(), 8);
  FAST_CHECK_EQ(product_traits<Rn, R3>::elem2(y).size(), 3);
}

TEST_CASE("DirecProduct")
{
  using R3 = manifold::Real<3>;
  using R4 = manifold::Real<4>;
  using P = manifold::DirectProduct<R3, R4>;

  typename P::repr_t x = P::repr_t::Random();
  P::repr_t y = P::repr_t::Random();
  MatrixXd M = MatrixXd::Random(3, 8);
  auto z = M.row(0).transpose().head(7);
  FAST_CHECK_EQ(P::dim, 7);
  FAST_CHECK_EQ(P::dynamicDim(x), 7);
  FAST_CHECK_UNARY((x + y).isApprox(P::compose(x, y)));
  FAST_CHECK_UNARY(x.isApprox(P::compose(x, P::identity)));
  FAST_CHECK_UNARY(y.isApprox(P::compose(P::identity, y)));
  FAST_CHECK_UNARY((z + x + y).isApprox(P::compose(z, x + y)));
  FAST_CHECK_UNARY((-x).isApprox(P::inverse(x)));
  FAST_CHECK_UNARY(std::is_same_v<decltype(R3::inverse(R3::identity)), R3::Identity>);
  
  auto l = P::log(x);
  FAST_CHECK_UNARY(x.isApprox(l));
  FAST_CHECK_UNARY(std::is_same_v<decltype(P::log(P::identity)), P::AlgIdentity>);
  FAST_CHECK_UNARY(x.isApprox(P::exp(l)));
  FAST_CHECK_UNARY(std::is_same_v<decltype(P::exp(P::algIdentity)), P::Identity>);
  FAST_CHECK_UNARY((x - y).isApprox(P::compose(P::exp(l), P::inverse(y))));
}

template<typename LG, typename Expr>
struct checkAdjoint
{
  template<typename Adj>
  static void run(const Adj& adj, bool inverse, bool transpose, bool sign,
    const typename LG::repr_t& opValue, const typename Adj::matrix_t& mat)
  {
    FAST_CHECK_UNARY(std::is_same_v<typename Adj::LG, LG>);
    FAST_CHECK_UNARY(std::is_same_v<typename Adj::Expr, Expr>);
    FAST_CHECK_EQ(Adj::Inverse, inverse);
    FAST_CHECK_EQ(Adj::Transpose, transpose);
    FAST_CHECK_EQ(Adj::PositiveSign, sign);
    if constexpr (!std::is_same_v<Expr,typename LG::Identity>)
      FAST_CHECK_UNARY(opValue.isApprox(adj.operand()));
    if constexpr (LG::dim>=0 || !Adj::template isId<typename Adj::Expr>::value)
      FAST_CHECK_UNARY(mat.isApprox(adj.matrix()));
    else
      FAST_CHECK_UNARY(mat.isApprox(adj.matrix(opValue.size())));
  }
};

TEST_CASE("AdjointR3")
{
  using R3 = manifold::Real<3>;
  using tvm::manifold::internal::ReprOwn;
  using tvm::manifold::internal::ReprRef;
  Vector3d x = Vector3d::Random();
  Vector3d y = Vector3d::Random();
  FAST_CHECK_UNARY(std::is_same_v<decltype(R3::adjoint(x))::Expr, ReprRef<R3>>);
  FAST_CHECK_UNARY(std::is_same_v<decltype(R3::adjoint(x+y))::Expr, ReprOwn<R3>>);
  FAST_CHECK_UNARY(std::is_same_v<decltype(R3::adjoint(R3::Identity{}))::Expr, R3::Identity>);
  tvm::manifold::internal::Adjoint<R3> AdX(x);
  tvm::manifold::internal::Adjoint<R3> AdY(y);
  tvm::manifold::internal::Adjoint AdI(R3::Identity{});
  auto AdXY = AdX * AdY;
  auto AdXI = AdX * AdI;
  auto AdIX = AdI * AdX;
  auto AdII = AdI * AdI;
  checkAdjoint<R3, ReprOwn<R3>>::run(AdXY, false, false, true, x + y, Matrix3d::Identity());
  checkAdjoint<R3, ReprOwn<R3>>::run(AdXI, false, false, true, x, Matrix3d::Identity());
  checkAdjoint<R3, ReprOwn<R3>>::run(AdIX, false, false, true, x, Matrix3d::Identity());
  checkAdjoint<R3, decltype(AdI)::Expr>::run(AdII, false, false, true, {}, Matrix3d::Identity());

  auto Ax_i = AdX.inverse();
  auto Ax_t = AdX.transpose();
  auto Ax_d = AdX.dual();
  auto Ax_m = -AdX;
  checkAdjoint<R3, ReprOwn<R3>>::run(Ax_i, true, false, true, x, Matrix3d::Identity());
  checkAdjoint<R3, ReprOwn<R3>>::run(Ax_t, false, true, true, x, Matrix3d::Identity());
  checkAdjoint<R3, ReprOwn<R3>>::run(Ax_d, true, true, true, x, Matrix3d::Identity());
  checkAdjoint<R3, ReprOwn<R3>>::run(Ax_m, false, false, false, x, -Matrix3d::Identity());
  auto Ay_i = AdY.inverse();
  auto Ay_m = -AdY;
  auto xi_yi = Ax_i * Ay_i;
  auto xi_ym = Ax_i * Ay_m;
  auto xm_yi = Ax_m * Ay_i;
  auto xm_ym = Ax_m * Ay_m;
  checkAdjoint<R3, ReprOwn<R3>>::run(xi_yi, true, false, true, y + x, Matrix3d::Identity());
  checkAdjoint<R3, ReprOwn<R3>>::run(xi_ym, false, false, false, y - x, -Matrix3d::Identity());
  checkAdjoint<R3, ReprOwn<R3>>::run(xm_yi, false, false, false, x - y, -Matrix3d::Identity());
  checkAdjoint<R3, ReprOwn<R3>>::run(xm_ym, false, false, true, x + y, Matrix3d::Identity());
}

TEST_CASE("AdjointRn")
{
  using Rn = manifold::Real<Dynamic>;
  using tvm::manifold::internal::ReprOwn;
  using tvm::manifold::internal::ReprRef;
  VectorXd x = VectorXd::Random(8);
  VectorXd y = VectorXd::Random(8);
  FAST_CHECK_UNARY(std::is_same_v<decltype(Rn::adjoint(x))::Expr, ReprRef<Rn>>);
  FAST_CHECK_UNARY(std::is_same_v<decltype(Rn::adjoint(x + y))::Expr, ReprOwn<Rn>>);
  FAST_CHECK_UNARY(std::is_same_v<decltype(Rn::adjoint(Rn::Identity{}))::Expr, Rn::Identity > );
  tvm::manifold::internal::Adjoint<Rn> AdX(x);
  tvm::manifold::internal::Adjoint<Rn> AdY(y);
  tvm::manifold::internal::Adjoint AdI(Rn::Identity{});
  auto AdXY = AdX * AdY;
  auto AdXI = AdX * AdI;
  auto AdIX = AdI * AdX;
  auto AdII = AdI * AdI;
  checkAdjoint<Rn, ReprOwn<Rn>>::run(AdXY, false, false, true, x + y, MatrixXd::Identity(8, 8));
  checkAdjoint<Rn, ReprOwn<Rn>>::run(AdXI, false, false, true, x, MatrixXd::Identity(8, 8));
  checkAdjoint<Rn, ReprOwn<Rn>>::run(AdIX, false, false, true, x, MatrixXd::Identity(8, 8));
  checkAdjoint<Rn, decltype(AdI)::Expr>::run(AdII, false, false, true, VectorXd::Zero(8), MatrixXd::Identity(8, 8));

  auto Ax_i = AdX.inverse();
  auto Ax_t = AdX.transpose();
  auto Ax_d = AdX.dual();
  auto Ax_m = -AdX;
  checkAdjoint<Rn, ReprOwn<Rn>>::run(Ax_i, true, false, true, x, MatrixXd::Identity(8,8));
  checkAdjoint<Rn, ReprOwn<Rn>>::run(Ax_t, false, true, true, x, MatrixXd::Identity(8,8));
  checkAdjoint<Rn, ReprOwn<Rn>>::run(Ax_d, true, true, true, x, MatrixXd::Identity(8,8));
  checkAdjoint<Rn, ReprOwn<Rn>>::run(Ax_m, false, false, false, x, -MatrixXd::Identity(8,8));
  auto Ay_i = AdY.inverse();
  auto Ay_m = -AdY;
  auto xi_yi = Ax_i * Ay_i;
  auto xi_ym = Ax_i * Ay_m;
  auto xm_yi = Ax_m * Ay_i;
  auto xm_ym = Ax_m * Ay_m;
  checkAdjoint<Rn, ReprOwn<Rn>>::run(xi_yi, true, false, true, y + x, MatrixXd::Identity(8,8));
  checkAdjoint<Rn, ReprOwn<Rn>>::run(xi_ym, false, false, false, y - x, -MatrixXd::Identity(8,8));
  checkAdjoint<Rn, ReprOwn<Rn>>::run(xm_yi, false, false, false, x - y, -MatrixXd::Identity(8,8));
  checkAdjoint<Rn, ReprOwn<Rn>>::run(xm_ym, false, false, true, x + y, MatrixXd::Identity(8,8));
}

TEST_CASE("AdjointSO3")
{
  using SO3 = manifold::SO3;
  using tvm::manifold::internal::ReprOwn;
  using tvm::manifold::internal::ReprRef;
  Matrix3d x = Quaterniond::UnitRandom().toRotationMatrix();
  Matrix3d y = Quaterniond::UnitRandom().toRotationMatrix();
  FAST_CHECK_UNARY(std::is_same_v<decltype(SO3::adjoint(x))::Expr, ReprRef<SO3>>);
  FAST_CHECK_UNARY(std::is_same_v<decltype(SO3::adjoint(x * y))::Expr, ReprOwn<SO3>>);
  FAST_CHECK_UNARY(std::is_same_v<decltype(SO3::adjoint(SO3::Identity{}))::Expr, SO3::Identity > );
  {
    tvm::manifold::internal::Adjoint<SO3> AdX(x);
    tvm::manifold::internal::Adjoint<SO3> AdY(y);
    tvm::manifold::internal::Adjoint AdI(SO3::Identity{});
    auto AdXY = AdX * AdY;
    auto AdXI = AdX * AdI;
    auto AdIX = AdI * AdX;
    auto AdII = AdI * AdI;
    checkAdjoint<SO3, ReprOwn<SO3>>::run(AdXY, false, false, true, x * y, x * y);
    checkAdjoint<SO3, ReprOwn<SO3>>::run(AdXI, false, false, true, x, x);
    checkAdjoint<SO3, ReprOwn<SO3>>::run(AdIX, false, false, true, x, x);
    checkAdjoint<SO3, decltype(AdI)::Expr>::run(AdII, false, false, true, {}, Matrix3d::Identity());

    auto Ax_i = AdX.inverse();
    auto Ax_t = AdX.transpose();
    auto Ax_d = AdX.dual();
    auto Ax_m = -AdX;
    checkAdjoint<SO3, ReprOwn<SO3>>::run(Ax_i, true, false, true, x, x.transpose());
    checkAdjoint<SO3, ReprOwn<SO3>>::run(Ax_t, false, true, true, x, x.transpose());
    checkAdjoint<SO3, ReprOwn<SO3>>::run(Ax_d, true, true, true, x, x);
    checkAdjoint<SO3, ReprOwn<SO3>>::run(Ax_m, false, false, false, x, -x);
    auto Ay_i = AdY.inverse();
    auto Ay_m = -AdY;
    auto xi_yi = Ax_i * Ay_i;
    auto xi_ym = Ax_i * Ay_m;
    auto xm_yi = Ax_m * Ay_i;
    auto xm_ym = Ax_m * Ay_m;
    checkAdjoint<SO3, ReprOwn<SO3>>::run(xi_yi, true, false, true, y * x, (y * x).transpose());
    checkAdjoint<SO3, ReprOwn<SO3>>::run(xi_ym, false, false, false, x.transpose() * y, -x.transpose() * y);
    checkAdjoint<SO3, ReprOwn<SO3>>::run(xm_yi, false, false, false, x * y.transpose(), -x * y.transpose());
    checkAdjoint<SO3, ReprOwn<SO3>>::run(xm_ym, false, false, true, x * y, x * y);
  }

  {
    tvm::manifold::internal::Adjoint<SO3, ReprRef<SO3>> AdX(x);
    tvm::manifold::internal::Adjoint<SO3> AdY(y);
    tvm::manifold::internal::Adjoint AdI(SO3::Identity{});
    auto AdXY = AdX * AdY;
    auto AdXI = AdX * AdI;
    auto AdIX = AdI * AdX;
    auto AdII = AdI * AdI;
    checkAdjoint<SO3, ReprOwn<SO3>>::run(AdXY, false, false, true, x * y, x * y);
    checkAdjoint<SO3, ReprRef<SO3>>::run(AdXI, false, false, true, x, x);
    checkAdjoint<SO3, ReprRef<SO3>>::run(AdIX, false, false, true, x, x);
    checkAdjoint<SO3, decltype(AdI)::Expr>::run(AdII, false, false, true, {}, Matrix3d::Identity());

    auto Ax_i = AdX.inverse();
    auto Ax_t = AdX.transpose();
    auto Ax_d = AdX.dual();
    auto Ax_m = -AdX;
    checkAdjoint<SO3, ReprRef<SO3>>::run(Ax_i, true, false, true, x, x.transpose());
    checkAdjoint<SO3, ReprRef<SO3>>::run(Ax_t, false, true, true, x, x.transpose());
    checkAdjoint<SO3, ReprRef<SO3>>::run(Ax_d, true, true, true, x, x);
    checkAdjoint<SO3, ReprRef<SO3>>::run(Ax_m, false, false, false, x, -x);
    auto Ay_i = AdY.inverse();
    auto Ay_m = -AdY;
    auto xi_yi = Ax_i * Ay_i;
    auto xi_ym = Ax_i * Ay_m;
    auto xm_yi = Ax_m * Ay_i;
    auto xm_ym = Ax_m * Ay_m;
    checkAdjoint<SO3, ReprOwn<SO3>>::run(xi_yi, true, false, true, y * x, (y * x).transpose());
    checkAdjoint<SO3, ReprOwn<SO3>>::run(xi_ym, false, false, false, x.transpose() * y, -x.transpose() * y);
    checkAdjoint<SO3, ReprOwn<SO3>>::run(xm_yi, false, false, false, x * y.transpose(), -x * y.transpose());
    checkAdjoint<SO3, ReprOwn<SO3>>::run(xm_ym, false, false, true, x * y, x * y);
  }
}

TEST_CASE("AdjointS3")
{
  using S3 = manifold::S3;
  using tvm::manifold::internal::ReprOwn;
  using tvm::manifold::internal::ReprRef;
  Quaterniond x = Quaterniond::UnitRandom();
  Quaterniond y = Quaterniond::UnitRandom();
  FAST_CHECK_UNARY(std::is_same_v<decltype(S3::adjoint(x))::Expr, ReprRef<S3>>);
  FAST_CHECK_UNARY(std::is_same_v<decltype(S3::adjoint(x * y))::Expr, ReprOwn<S3>>);
  FAST_CHECK_UNARY(std::is_same_v<decltype(S3::adjoint(S3::Identity{}))::Expr, S3::Identity > );
  tvm::manifold::internal::Adjoint<S3> AdX(x);
  tvm::manifold::internal::Adjoint<S3> AdY(y);
  tvm::manifold::internal::Adjoint AdI(S3::Identity{});
  auto AdXY = AdX * AdY;
  auto AdXI = AdX * AdI;
  auto AdIX = AdI * AdX;
  auto AdII = AdI * AdI;
  checkAdjoint<S3, ReprOwn<S3>>::run(AdXY, false, false, true, x * y, (x * y).toRotationMatrix());
  checkAdjoint<S3, ReprOwn<S3>>::run(AdXI, false, false, true, x, x.toRotationMatrix());
  checkAdjoint<S3, ReprOwn<S3>>::run(AdIX, false, false, true, x, x.toRotationMatrix());
  checkAdjoint<S3, decltype(AdI)::Expr>::run(AdII, false, false, true, {}, Matrix3d::Identity());

  auto Ax_i = AdX.inverse();
  auto Ax_t = AdX.transpose();
  auto Ax_d = AdX.dual();
  auto Ax_m = -AdX;
  checkAdjoint<S3, ReprOwn<S3>>::run(Ax_i, true, false, true, x, x.toRotationMatrix().transpose());
  checkAdjoint<S3, ReprOwn<S3>>::run(Ax_t, false, true, true, x, x.toRotationMatrix().transpose());
  checkAdjoint<S3, ReprOwn<S3>>::run(Ax_d, true, true, true, x, x.toRotationMatrix());
  checkAdjoint<S3, ReprOwn<S3>>::run(Ax_m, false, false, false, x, -x.toRotationMatrix());
  auto Ay_i = AdY.inverse();
  auto Ay_m = -AdY;
  auto xi_yi = Ax_i * Ay_i;
  auto xi_ym = Ax_i * Ay_m;
  auto xm_yi = Ax_m * Ay_i;
  auto xm_ym = Ax_m * Ay_m;
  checkAdjoint<S3, ReprOwn<S3>>::run(xi_yi, true, false, true, y * x, (y * x).toRotationMatrix().transpose());
  checkAdjoint<S3, ReprOwn<S3>>::run(xi_ym, false, false, false, x.inverse() * y, -x.toRotationMatrix().transpose() * y);
  checkAdjoint<S3, ReprOwn<S3>>::run(xm_yi, false, false, false, x * y.inverse(), -x.toRotationMatrix() * y.toRotationMatrix().transpose());
  checkAdjoint<S3, ReprOwn<S3>>::run(xm_ym, false, false, true, x * y, (x * y).toRotationMatrix());
}

TEST_CASE("log-exp on rotations")
{
  using SO3 = manifold::SO3;
  using S3 = manifold::S3;

  Vector3d v = Vector3d::Random().normalized();
  double h = 1;
  for (int i = 0; i < 18; ++i)
  {
    Vector3d t = h * v;
    Quaterniond q = S3::exp(t);
    Matrix3d R = SO3::exp(t);
    Matrix3d R0 = SO3::hat(t).exp();
    FAST_CHECK_UNARY(R0.isApprox(R));
    FAST_CHECK_UNARY(R0.isApprox(q.toRotationMatrix()));
    FAST_CHECK_UNARY(t.isApprox(SO3::log(R)));
    FAST_CHECK_UNARY(t.isApprox(S3::log(q)));
    h /= 10;
  }
}

template <typename LG>
void checkJacobians()
{
  static_assert(LG::dim >= 0);
  double h = 1e-8;
  VectorXd t = VectorXd::Random(LG::dim);
  auto e0 = LG::exp(t);
  typename LG::jacobian_t Jr0 = LG::rightJacobian(t);
  typename LG::jacobian_t Jl0 = LG::leftJacobian(t);
  typename LG::jacobian_t Jr, Jl;

  //finite differences
  for (int i = 0; i < LG::dim; ++i)
  {
    t[i] += h;
    Jr.col(i) = LG::log(LG::compose(LG::inverse(e0), LG::exp(t))) / h; //log(exp(t)^-1*exp(t+h))/h (right jacobian of exp)
    Jl.col(i) = LG::log(LG::compose(LG::exp(t), LG::inverse(e0))) / h; //log(exp(t+h)*exp(t)^-1)/h (left jacobian of exp)
    t[i] -= h;
  }

  FAST_CHECK_UNARY(Jr0.isApprox(Jr, 1e-6));
  FAST_CHECK_UNARY(Jl0.isApprox(Jl, 1e-6));
  FAST_CHECK_UNARY(LG::invRightJacobian(t).isApprox(Jr0.inverse()));
  FAST_CHECK_UNARY(LG::invLeftJacobian(t).isApprox(Jl0.inverse()));
  FAST_CHECK_UNARY(LG::adjoint(e0).matrix().isApprox(Jl0 * Jr0.inverse()));  // Checking identity Ad_{exp(t)} = Jl(t)*Jr^-1
}

TEST_CASE("Manifold jacobian")
{
  using namespace tvm::manifold;
  checkJacobians<Real<3>>();
  checkJacobians<SO3>();
  checkJacobians<S3>();
}
