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
// This file defines a collection of statistics collectors.

#pragma once

#include "yb/rocksdb/table_properties.h"

#include <memory>
#include <string>
#include <vector>

namespace rocksdb {

struct InternalKeyTablePropertiesNames {
  static const std::string kDeletedKeys;
};

// Base class for internal table properties collector.
class IntTblPropCollector {
 public:
  virtual ~IntTblPropCollector() {}
  virtual Status Finish(UserCollectedProperties* properties) = 0;

  virtual const char* Name() const = 0;

  // @params key    the user key that is inserted into the table.
  // @params value  the value that is inserted into the table.
  virtual Status InternalAdd(const Slice& key, const Slice& value,
                             uint64_t file_size) = 0;

  virtual UserCollectedProperties GetReadableProperties() const = 0;

  virtual bool NeedCompact() const { return false; }
};

// Factory for internal table properties collector.
class IntTblPropCollectorFactory {
 public:
  virtual ~IntTblPropCollectorFactory() {}
  // has to be thread-safe
  virtual IntTblPropCollector* CreateIntTblPropCollector(
      uint32_t column_family_id) = 0;

  // The name of the properties collector can be used for debugging purpose.
  virtual const char* Name() const = 0;
};

typedef std::vector<std::unique_ptr<IntTblPropCollectorFactory>> IntTblPropCollectorFactories;

// Collecting the statistics for internal keys. Visible only by internal
// rocksdb modules.
class InternalKeyPropertiesCollector : public IntTblPropCollector {
 public:
  virtual Status InternalAdd(const Slice& key, const Slice& value,
                             uint64_t file_size) override;

  virtual Status Finish(UserCollectedProperties* properties) override;

  virtual const char* Name() const override {
    return "InternalKeyPropertiesCollector";
  }

  UserCollectedProperties GetReadableProperties() const override;

 private:
  uint64_t deleted_keys_ = 0;
};

class InternalKeyPropertiesCollectorFactory
    : public IntTblPropCollectorFactory {
 public:
  virtual IntTblPropCollector* CreateIntTblPropCollector(
      uint32_t column_family_id) override {
    return new InternalKeyPropertiesCollector();
  }

  virtual const char* Name() const override {
    return "InternalKeyPropertiesCollectorFactory";
  }
};

// When rocksdb creates a new table, it will encode all "user keys" into
// "internal keys", which contains meta information of a given entry.
//
// This class extracts user key from the encoded internal key when Add() is
// invoked.
class UserKeyTablePropertiesCollector : public IntTblPropCollector {
 public:
  // transfer of ownership
  explicit UserKeyTablePropertiesCollector(TablePropertiesCollector* collector)
      : collector_(collector) {}

  virtual ~UserKeyTablePropertiesCollector() {}

  virtual Status InternalAdd(const Slice& key, const Slice& value,
                             uint64_t file_size) override;

  virtual Status Finish(UserCollectedProperties* properties) override;

  virtual const char* Name() const override { return collector_->Name(); }

  UserCollectedProperties GetReadableProperties() const override;

  virtual bool NeedCompact() const override {
    return collector_->NeedCompact();
  }

 protected:
  std::unique_ptr<TablePropertiesCollector> collector_;
};

class UserKeyTablePropertiesCollectorFactory
    : public IntTblPropCollectorFactory {
 public:
  explicit UserKeyTablePropertiesCollectorFactory(
      std::shared_ptr<TablePropertiesCollectorFactory> user_collector_factory)
      : user_collector_factory_(user_collector_factory) {}
  virtual IntTblPropCollector* CreateIntTblPropCollector(
      uint32_t column_family_id) override {
    TablePropertiesCollectorFactory::Context context;
    context.column_family_id = column_family_id;
    return new UserKeyTablePropertiesCollector(
        user_collector_factory_->CreateTablePropertiesCollector(context));
  }

  virtual const char* Name() const override {
    return user_collector_factory_->Name();
  }

 private:
  std::shared_ptr<TablePropertiesCollectorFactory> user_collector_factory_;
};

}  // namespace rocksdb

