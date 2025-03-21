/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright (C) 2013-2022 Synaptics Incorporated. All rights reserved.
 *
 * This file contains information that is proprietary to Synaptics
 * Incorporated ("Synaptics"). The holder of this file shall treat all
 * information contained herein as confidential, shall use the
 * information only for its intended purpose, and shall not duplicate,
 * disclose, or disseminate any of this information in any manner
 * unless Synaptics has otherwise provided express, written
 * permission.
 *
 * Use of the materials may require a license of intellectual property
 * from a third party or from Synaptics. This file conveys no express
 * or implied licenses to any intellectual property rights belonging
 * to Synaptics.
 *
 * INFORMATION CONTAINED IN THIS DOCUMENT IS PROVIDED "AS-IS", AND
 * SYNAPTICS EXPRESSLY DISCLAIMS ALL EXPRESS AND IMPLIED WARRANTIES,
 * INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE, AND ANY WARRANTIES OF NON-INFRINGEMENT OF ANY
 * INTELLECTUAL PROPERTY RIGHTS. IN NO EVENT SHALL SYNAPTICS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, PUNITIVE, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OF THE INFORMATION CONTAINED IN THIS DOCUMENT, HOWEVER CAUSED AND
 * BASED ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, AND EVEN IF SYNAPTICS WAS
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. IF A TRIBUNAL OF
 * COMPETENT JURISDICTION DOES NOT PERMIT THE DISCLAIMER OF DIRECT
 * DAMAGES OR ANY OTHER DAMAGES, SYNAPTICS' TOTAL CUMULATIVE LIABILITY
 * TO ANY PARTY SHALL NOT EXCEED ONE HUNDRED U.S. DOLLARS.
 */
///
/// Application-level information for AI model.
///

#pragma once

#include <string>
#include <vector>


namespace synaptics {
namespace synap {


/// Application-level information for Classification and Object Detection
class LabelInfo {
public:
    ///  Default constructor.
    LabelInfo();

    ///  Construct with a file.
    ///
    ///  @param filename: name of json file containing application-level information
    LabelInfo(const std::string& filename);

    ///  Init with a file.
    ///
    ///  @param filename: name of json file containing application-level information
    bool init(const std::string& filename);

    ///  Init with a file.
    ///  (Avoid relying on C++ STL types as arguments for compatibility with other lib versions)
    ///
    ///  @param filename: name of json file containing application-level information
    bool init(const char* filename);

    ///  Get label.
    ///
    ///  @param class_index: index of the class
    ///  @return class label if available else empty string
    const std::string& label(int class_index) const;

    ///  Get label as char*.
    ///  (Avoid relying on C++ STL types as arguments for compatibility with other lib versions)
    ///
    ///  @param class_index: index of the class
    ///  @return class label if available else empty string
    const char* label_ptr(int class_index) const;

private:
    std::vector<std::string> _labels;
};

}  // namespace synap
}  // namespace synaptics
