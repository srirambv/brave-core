/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_BASE_BRAVE_SHIELDS_SERVICE_H_
#define BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_BASE_BRAVE_SHIELDS_SERVICE_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>
#include <mutex>

#include "base/files/file_path.h"
#include "base/sequenced_task_runner.h"
#include "content/public/common/resource_type.h"
#include "url/gurl.h"

namespace brave_shields {

// The brave shields service in charge of checking brave shields like ad-block,
// tracking protection, etc.
class BaseBraveShieldsService {
 public:
  BaseBraveShieldsService(const std::string& updater_name,
                          const std::string& updater_id,
                          const std::string& updater_base64_public_key);
  virtual ~BaseBraveShieldsService();
  bool Start();
  void Stop();
  bool IsInitialized() const;
  virtual bool ShouldStartRequest(const GURL& url,
      content::ResourceType resource_type,
      const std::string& tab_host);
  scoped_refptr<base::SequencedTaskRunner> GetTaskRunner() {
    return task_runner_;
  }

 protected:
  virtual bool Init() = 0;
  virtual void Cleanup() = 0;
  virtual void OnComponentRegistered(const std::string& extension_id) = 0;
  virtual void OnComponentReady(const std::string& extension_id,
                                const base::FilePath& install_dir) = 0;

 private:
  void InitShields();

  bool initialized_;
  std::mutex initialized_mutex_;
  scoped_refptr<base::SequencedTaskRunner> task_runner_;
  std::string updater_name_;
  std::string updater_id_;
  std::string updater_base64_public_key_;
};

}  // namespace brave_shields

#endif  // BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_BASE_BRAVE_SHIELDS_SERVICE_H_
