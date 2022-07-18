//
// Created by joseph on 7/18/22.
//

#include "fdeep/fdeep.hpp"

int main(int argc, char** argv) {
    const auto model = fdeep::load_model("fdeep_model.json");
    const auto result = model.predict(
            {fdeep::tensor(fdeep::tensor_shape(static_cast<std::size_t>(4)),
                           std::vector<float>{1, 2, 3, 4})});
    std::cout << fdeep::show_tensors(result) << std::endl;
}

