#include "problems/erikkson/erikkson_mumps.hpp"
#include "problems/erikkson/erikkson_supg.hpp"
#include "problems/erikkson/erikkson_cg.hpp"
// #include "problems/erikkson/erikkson_quanling.hpp"
// #include "problems/erikkson/pollution_cg.hpp"
// #include "problems/erikkson/erikkson_mumps_split.hpp"


using namespace ads;

double shishkin_const(int n, double eps) {
    return std::log(n) * eps;
}

bspline::basis create_basis(double a, double b, int p, int elements, int repeated_nodes, bool adapt) {
    int points = elements + 1;
    int r = repeated_nodes + 1;
    int knot_size = 2 * (p + 1) + (points - 2) * r;
    bspline::knot_vector knot(knot_size);

    for (int i = 0; i <= p; ++i) {
        knot[i] = a;
        knot[knot_size - i - 1] = b;
    }

    auto x0 = 0.5;
    // auto d = 0.0000046;
    // auto d = 0.0368;

    // auto d = shishkin_const(elements, 1e-6);
    auto d = 0.01;

    std::cout << "Shishkin: " << d << std::endl;
    auto y0 = 1 - d;

    for (int i = 1; i < points - 1; ++i) {
        auto t = lerp(i, elements, 0.0, 1.0);

        auto s = adapt ? (t < x0 ? t / x0 * y0 : (t - x0) / (1 - x0) * (1 - y0) + y0) : t;
        for (int j = 0; j < r; ++ j) {
            knot[p + 1 + (i - 1) * r + j] = lerp(s, a, b);
        }
    }

    return {std::move(knot), p};
}


int main(int argc, char* argv[]) {
    if (argc != 9) {
        std::cerr << "Usage: erikkson_mumps <N> <subdivision> <adapt> <p_trial> <C_trial> <p_test> <C_test> <steps>" << std::endl;
        std::exit(1);
    }
    int n = std::atoi(argv[1]);
    int subdivision = std::atoi(argv[2]);
    bool adapt = std::atoi(argv[3]);

    int p_trial = std::atoi(argv[4]);
    int C_trial = std::atoi(argv[5]);
    int p_test = std::atoi(argv[6]);
    int C_test = std::atoi(argv[7]);
    int nsteps = std::atoi(argv[8]);

    // double S = 5000.0;
    double S = 1.0;

    int quad = std::max(p_trial, p_test) + 1;
    dim_config trial{ p_trial, n, 0.0, S, quad, p_trial - 1 - C_trial };
    dim_config test { p_test,  n, 0.0, S, quad, p_test  - 1 - C_test };

    std::cout << "adaptations: " << std::boolalpha << adapt << std::endl;


    timesteps_config steps{ nsteps, 1e-2 };
    int ders = 2;
    // int subdivision = 2;
    // int adapt = 0;

    auto trial_basis_x = create_basis(0, S, p_trial, n, p_trial - 1 - C_trial, adapt);
    // auto trial_basis_x = create_adapted_basis(0, 1, p_trial, p_trial - 1 - C_trial);

    auto dtrial_x = dimension{ trial_basis_x, quad, ders, subdivision };

    // auto trial_basis_y = bspline::create_basis(0, S, p_trial, n, p_trial - 1 - C_trial);
    auto trial_basis_y = create_basis(0, S, p_trial, n, p_trial - 1 - C_trial, adapt);

    auto dtrial_y = dimension{ trial_basis_y, quad, ders, subdivision };

    auto test_basis_x = create_basis(0, S, p_test, subdivision*n, p_test - 1 - C_test, adapt);
    // auto test_basis_x = create_adapted_basis(0, 1, p_test, p_test - 1 - C_test);

    auto dtest_x = dimension{ test_basis_x, quad, ders, 1 };

    // auto test_basis_y = bspline::create_basis(0, S, p_test, subdivision*n, p_test - 1 - C_test);
    auto test_basis_y = create_basis(0, S, p_test, subdivision*n, p_test - 1 - C_test, adapt);

    auto dtest_y = dimension{ test_basis_y, quad, ders, 1 };

    auto trial_dim = dtrial_x.B.dofs();
    auto test_dim = dtest_x.B.dofs();

    if (trial_dim > test_dim) {
        std::cerr << "Dimension of the trial space greater than that of test space ("
                  << trial_dim << " > " << test_dim << ")" << std::endl;
        std::exit(1);
    } else {
        std::cout << "dim(U) = " << trial_dim << ", dim(V) = " << test_dim << std::endl;
    }

    // erikkson_mumps_split sim{dtrial_x, dtrial_y, dtest_x, dtest_y, steps};
    // erikkson_CG sim{dtrial_x, dtrial_y, dtest_x, dtest_y, steps};
    // pollution_CG sim{dtrial_x, dtrial_y, dtest_x, dtest_y, steps};
    // erikkson_quanling sim{dtrial_x, dtrial_y, dtest_x, dtest_y, steps};
    // erikkson_mumps sim{dtrial_x, dtrial_y, dtest_x, dtest_y, steps};
    erikkson_supg sim{dtrial_x, dtrial_y, dtest_x, dtest_y, steps};

    sim.run();
}
