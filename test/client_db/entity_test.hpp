#pragma once

#include <gtest/gtest.h>
#include <vector>

#include "client_db/client_db.hpp"
#include "utils/randomizer.hpp"

class EntityTest : public ::testing::Test {
 protected:
  Randomizer randomizer;
  ClientDb db;
  int sqlite_code = 0;

  EntityTest() : db(DEV_DEFAULT_CLIENT_DB_PATH, false) {}

  virtual ~EntityTest() {}

  virtual void SetUp() {
    ASSERT_TRUE(db.clear(&sqlite_code));
    ASSERT_TRUE(db.is_sqlite_success(sqlite_code));

    ASSERT_TRUE(db.init(&sqlite_code));
    ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
  }

  virtual void TearDown() {
    ASSERT_TRUE(db.clear(&sqlite_code));
    ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
  }

  template <typename TEntity, typename TEntityDescriptor>
  void test_CreateAndRead_Success() {
    TEntityDescriptor descriptor;
    TEntity entity;
    descriptor.randomize(entity);

    // create one
    ASSERT_TRUE(descriptor.create_entity(db, entity, &sqlite_code));
    ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
    ASSERT_EQ(1, entity.sqlite_id);

    // read one
    TEntity entity_copy;
    ASSERT_TRUE(descriptor.read_entity(db, entity.sqlite_id, entity_copy,
                                       &sqlite_code));
    ASSERT_TRUE(db.is_sqlite_success(sqlite_code));

    // read one equality
    descriptor.assert_equal(entity, entity_copy);
  }

  template <typename TEntity, typename TEntityDescriptor>
  void test_InvalidRead_DoesNotChangeInputObject() {
    TEntityDescriptor descriptor;
    TEntity entity;
    descriptor.randomize(entity);

    // create one
    ASSERT_TRUE(descriptor.create_entity(db, entity, &sqlite_code));
    ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
    ASSERT_EQ(1, entity.sqlite_id);

    // read one
    TEntity entity_copy;
    ASSERT_TRUE(descriptor.read_entity(db, entity.sqlite_id, entity_copy,
                                       &sqlite_code));
    ASSERT_TRUE(db.is_sqlite_success(sqlite_code));

    // read one id not found, sql success
    entity_copy.sqlite_id = -1;
    ASSERT_FALSE(descriptor.read_entity(db, 10, entity_copy, &sqlite_code));
    ASSERT_TRUE(db.is_sqlite_success(sqlite_code));

    // read one invalid sql id
    ASSERT_THROW(descriptor.read_entity(db, -1, entity_copy, &sqlite_code),
                 ClientDbException);

    // object equality after failed reads
    descriptor.assert_equal(entity, entity_copy, true);
  }

  template <typename TEntity, typename TEntityDescriptor>
  void test_UpdateAndRead_Success() {
    TEntityDescriptor descriptor;
    TEntity entity;
    descriptor.randomize(entity);

    // create one
    ASSERT_TRUE(descriptor.create_entity(db, entity, &sqlite_code));
    ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
    ASSERT_EQ(1, entity.sqlite_id);

    // read one
    TEntity entity_copy;
    ASSERT_TRUE(descriptor.read_entity(db, entity.sqlite_id, entity_copy,
                                       &sqlite_code));
    ASSERT_TRUE(db.is_sqlite_success(sqlite_code));

    // read one equality
    descriptor.assert_equal(entity, entity_copy);

    // update one
    descriptor.randomize(entity);
    ASSERT_TRUE(descriptor.update_entity(db, entity, &sqlite_code));
    ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
    ASSERT_EQ(1, db.changes());

    // read one after update
    ASSERT_TRUE(descriptor.read_entity(db, entity.sqlite_id, entity_copy,
                                       &sqlite_code));
    ASSERT_TRUE(db.is_sqlite_success(sqlite_code));

    // read one equality after update
    descriptor.assert_equal(entity, entity_copy);
  }

  template <typename TEntity, typename TEntityDescriptor>
  void test_InvalidUpdate_Exception() {
    TEntity entity;
    TEntityDescriptor descriptor;
    // update one invalid sql id
    entity.sqlite_id = -1;
    ASSERT_THROW(descriptor.update_entity(db, entity, &sqlite_code),
                 ClientDbException);
  }

  template <typename TEntity, typename TEntityDescriptor>
  void test_Delete_Success() {
    TEntity entity;
    TEntityDescriptor descriptor;
    descriptor.randomize(entity);

    // create one
    ASSERT_TRUE(descriptor.create_entity(db, entity, &sqlite_code));
    ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
    ASSERT_EQ(1, entity.sqlite_id);

    // delete one
    ASSERT_TRUE(descriptor.delete_entity(db, entity, &sqlite_code));
    ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
    ASSERT_EQ(1, db.changes());

    std::vector<TEntity> entities;
    ASSERT_TRUE(descriptor.read_entities(db, entities, &sqlite_code));
    ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
    ASSERT_EQ(0u, entities.size());
  }

  template <typename TEntity, typename TEntityDescriptor>
  void test_DeleteUnknownId_Success() {
    TEntity entity;
    TEntityDescriptor descriptor;
    descriptor.randomize(entity);

    // delete one not found
    entity.sqlite_id = 10;
    ASSERT_TRUE(descriptor.delete_entity(db, entity, &sqlite_code));
    ASSERT_EQ(0, db.changes());
  }

  template <typename TEntity, typename TEntityDescriptor>
  void test_DeleteInvalidId_Exception() {
    TEntity entity;
    TEntityDescriptor descriptor;
    descriptor.randomize(entity);

    // delete one invalid sql id
    entity.sqlite_id = -1;
    ASSERT_THROW(descriptor.delete_entity(db, entity, &sqlite_code),
                 ClientDbException);
  }

  template <typename TEntity, typename TEntityDescriptor>
  void test_CreateAndReadMany_Success() {
    std::vector<TEntity> entities;
    TEntityDescriptor descriptor;

    // create 20 entities
    for (int i = 0; i < 20; ++i) {
      TEntity entity;
      descriptor.randomize(entity);
      ASSERT_TRUE(descriptor.create_entity(db, entity, &sqlite_code));
      ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
      ASSERT_EQ(1 + i, entity.sqlite_id);
      entities.push_back(entity);
    }

    // read 20 entities
    std::vector<TEntity> entity_copies;
    ASSERT_TRUE(descriptor.read_entities(db, entity_copies, &sqlite_code));
    ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
    ASSERT_EQ(20u, entity_copies.size());
    ASSERT_EQ(entities.size(), entity_copies.size());

    for (int i = 0; i < 20; ++i) {
      descriptor.assert_equal(entities[i], entity_copies[i]);
    }
  }

  template <typename TEntity, typename TEntityDescriptor>
  void test_UpdateAndReadMany_Success() {
    std::vector<TEntity> entities;
    TEntityDescriptor descriptor;

    // create 20 entities
    for (int i = 0; i < 20; ++i) {
      TEntity entity;
      descriptor.randomize(entity);
      ASSERT_TRUE(descriptor.create_entity(db, entity, &sqlite_code));
      ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
      ASSERT_EQ(1 + i, entity.sqlite_id);
      entities.push_back(entity);
    }

    // update 20 entities
    for (int i = 0; i < 20; ++i) {
      descriptor.randomize(entities[i]);
      ASSERT_TRUE(descriptor.update_entity(db, entities[i], &sqlite_code));
      ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
    }

    // read 20 entities
    std::vector<TEntity> entity_copies;
    ASSERT_TRUE(descriptor.read_entities(db, entity_copies, &sqlite_code));
    ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
    ASSERT_EQ(20u, entity_copies.size());
    ASSERT_EQ(entities.size(), entity_copies.size());

    for (int i = 0; i < 20; ++i) {
      descriptor.assert_equal(entities[i], entity_copies[i]);
    }
  }

  template <typename TEntity, typename TEntityDescriptor>
  void test_DeleteMany_Success() {
    std::vector<TEntity> entities;
    TEntityDescriptor descriptor;

    // create 20 entities
    for (int i = 0; i < 20; ++i) {
      TEntity entity;
      descriptor.randomize(entity);
      ASSERT_TRUE(descriptor.create_entity(db, entity, &sqlite_code));
      ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
      ASSERT_EQ(1 + i, entity.sqlite_id);
      entities.push_back(entity);
    }

    // delete 20 entities
    for (int i = 0; i < 20; ++i) {
      ASSERT_TRUE(descriptor.delete_entity(db, entities[i], &sqlite_code));
      ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
    }

    std::vector<TEntity> entity_copies;
    ASSERT_TRUE(descriptor.read_entities(db, entity_copies, &sqlite_code));
    ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
    ASSERT_EQ(0u, entity_copies.size());
  }
};

#define ENTITY_TEST(Entity, Test)              \
  TEST_F(EntityTest, Entity##_##Test) {        \
    test_##Test<Entity, Entity##Descriptor>(); \
  }

#define ALL_ENTITY_TESTS(Entity)                            \
  ENTITY_TEST(Entity, CreateAndRead_Success)                \
  ENTITY_TEST(Entity, InvalidRead_DoesNotChangeInputObject) \
  ENTITY_TEST(Entity, UpdateAndRead_Success)                \
  ENTITY_TEST(Entity, InvalidUpdate_Exception)              \
  ENTITY_TEST(Entity, DeleteUnknownId_Success)              \
  ENTITY_TEST(Entity, Delete_Success)                       \
  ENTITY_TEST(Entity, DeleteInvalidId_Exception)            \
  ENTITY_TEST(Entity, CreateAndReadMany_Success)            \
  ENTITY_TEST(Entity, UpdateAndReadMany_Success)            \
  ENTITY_TEST(Entity, DeleteMany_Success)
