#ifndef ADS_PROBLEMS_SCALABILITY_TEST_2D_HPP_
#define ADS_PROBLEMS_SCALABILITY_TEST_2D_HPP_

#include "ads/simulation.hpp"
#include "ads/executor/galois.hpp"

namespace ads {
namespace problems {


class scalability_2d : public simulation_2d {
private:
    using Base = simulation_2d;
    vector_type u, u_prev;

    Galois::StatTimer integration_timer{"integration"};
    galois_executor executor;

public:
    scalability_2d(const config_2d& config, int threads)
    : Base{ config }
    , u{ shape() }
    , u_prev{ shape() }
    , executor{threads}
    { }

    double init_state(double x, double y) {
        double dx = x - 0.5;
        double dy = y - 0.5;
        double r2 = std::min(8 * (dx * dx + dy * dy), 1.0);
        return (r2 - 1) * (r2 - 1) * (r2 + 1) * (r2 + 1);
    };

private:
    void solve(vector_type& v) {
        Base::solve(v);
    }

    void prepare_matrices() {
        x.fix_left();
        Base::prepare_matrices();
    }

    void before() override {
        prepare_matrices();

        auto init = [this](double x, double y) { return init_state(x, y); };
        projection(u, init);
        solve(u);
    }

    void before_step(int /*iter*/, double /*t*/) override {
        using std::swap;
        swap(u, u_prev);
    }

    void step(int /*iter*/, double /*t*/) override {
        compute_rhs();
        solve(u);
    }

    double forcing(double x, double y) const {
        double dx = x - 0.5;
        double dy = y - 0.5;
        double r = std::sqrt(dx * dx + dy * dy);
        return std::exp(- r);
    }

    void compute_rhs() {
        integration_timer.start();
        auto& rhs = u;

        zero(rhs);

        executor.for_each(elements(), [&](index_type e) {
            auto U = element_rhs();

            double J = jacobian(e);
            for (auto q : quad_points()) {
                double w = weigth(q);
                auto x = point(e, q);
                value_type u = eval_fun(u_prev, e, q);

                for (auto a : dofs_on_element(e)) {
                    auto aa = dof_global_to_local(e, a);
                    value_type v = eval_basis(e, q, a);

                    double gradient_prod = grad_dot(u, v);
                    double val = u.val * v.val - steps.dt * (gradient_prod - forcing(x[0], x[1]));
                    U(aa[0], aa[1]) += val * w * J;
                }
            }

            executor.synchronized([&]() {
                update_global_rhs(rhs, U, e);
            });
        });
        integration_timer.stop();
    }

    virtual void after() override {
        auto total = static_cast<double>(integration_timer.get());
        auto avg = total / steps.step_count;
        std::cout << "{ 'integration' : " << avg  << "}" << std::endl;
    }
};

}
}




#endif /* ADS_PROBLEMS_SCALABILITY_TEST_2D_HPP_ */
