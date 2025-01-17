//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.
//
// The following only applies to changes made to this file as part of YugaByte development.
//
// Portions Copyright (c) YugaByte, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
// in compliance with the License.  You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied.  See the License for the specific language governing permissions and limitations
// under the License.
//
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//


#pragma once
#include "yb/rocksdb/env.h"

namespace rocksdb {

class TableCache;
class VersionStorageInfo;
class VersionEdit;
struct FileMetaData;
class InternalStats;

// A helper class so we can efficiently apply a whole sequence
// of edits to a particular state without creating intermediate
// Versions that contain full copies of the intermediate state.
class VersionBuilder {
 public:
  VersionBuilder(const EnvOptions& env_options, TableCache* table_cache,
                 VersionStorageInfo* base_vstorage, Logger* info_log = nullptr);
  ~VersionBuilder();
  void CheckConsistency(VersionStorageInfo* vstorage);
  void CheckConsistencyForDeletes(VersionEdit* edit, uint64_t number,
                                  int level);
  void Apply(VersionEdit* edit);
  void SaveTo(VersionStorageInfo* vstorage);
  void LoadTableHandlers(InternalStats* internal_stats, int max_threads = 1);
  void MaybeAddFile(VersionStorageInfo* vstorage, int level, FileMetaData* f);

 private:
  class Rep;
  Rep* rep_;
};

extern bool NewestFirstBySeqNo(FileMetaData* a, FileMetaData* b);

}  // namespace rocksdb

