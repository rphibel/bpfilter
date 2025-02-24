/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023 Meta Platforms, Inc. and affiliates.
 */

#include "core/list.c"

#include "harness/cmocka.h"
#include "harness/mock.h"

static void noop_free(void **data)
{
    UNUSED(data);
}

static void dummy_free(void **data)
{
    free(*data);
    *data = NULL;
}

static int _dummy_filler(bf_list *l, void *data,
                         int (*add)(bf_list *l, void *data))
{
    _cleanup_free_ int *_data;
    int r;

    _data = malloc(sizeof(*_data));
    if (!_data)
        return -ENOMEM;

    *_data = *(int *)data;

    r = add(l, _data);
    if (r < 0)
        return r;

    TAKE_PTR(_data);

    return 0;
}

static int dummy_filler_head(bf_list *l, void *data)
{
    return _dummy_filler(l, data, bf_list_add_head);
}

static int dummy_filler_tail(bf_list *l, void *data)
{
    return _dummy_filler(l, data, bf_list_add_tail);
}

static void new_and_fill(bf_list **l, size_t count, const bf_list_ops *ops,
                         int (*filler)(bf_list *l, void *data))
{
    assert_int_equal(0, bf_list_new(l, ops));

    for (size_t i = 1; i <= count; ++i)
        assert_int_equal(0, filler(*l, &i));

    assert_int_equal(count, bf_list_size(*l));
}

static void init_and_fill(bf_list *l, size_t count, const bf_list_ops *ops,
                          int (*filler)(bf_list *l, void *data))
{
    bf_list_init(l, ops);

    for (size_t i = 1; i <= count; ++i)
        assert_int_equal(0, filler(l, &i));

    assert_int_equal(count, bf_list_size(l));
}

static bf_list_ops noop_ops = {.free = noop_free};
static bf_list_ops dummy_ops = {.free = dummy_free};

/* TestAssert(list, bf_list_new, (NULL, NOT_NULL));
TestAssert(list, bf_list_new, (NOT_NULL, NULL));
TestAssert(list, bf_list_free, (NULL));
TestAssert(list, bf_list_init, (NULL, NOT_NULL));
TestAssert(list, bf_list_init, (NOT_NULL, NULL));
TestAssert(list, bf_list_clean, (NULL));
TestAssert(list, bf_list_size, (NULL));
TestAssert(list, bf_list_add_head, (NULL, NOT_NULL));
TestAssert(list, bf_list_add_tail, (NULL, NOT_NULL));
TestAssert(list, bf_list_get_head, (NULL));
TestAssert(list, bf_list_get_tail, (NULL));
TestAssert(list, bf_list_node_next, (NULL));
TestAssert(list, bf_list_node_prev, (NULL));
TestAssert(list, bf_list_node_get_data, (NULL));
TestAssert(list, bf_list_node_take_data, (NULL)); */

Test(list, new_and_free)
{
    bf_list *l = NULL;

    {
        // With noop operators
        assert_int_equal(0, bf_list_new(&l, &noop_ops));
        assert_int_equal(0, l->len);
        assert_null(l->head);
        assert_null(l->tail);

        bf_list_free(&l);
        assert_null(l);

        new_and_fill(&l, 3, &noop_ops, bf_list_add_head);
        assert_int_equal(3, l->len);
        assert_non_null(l->head);
        assert_non_null(l->tail);

        bf_list_free(&l);
        assert_null(l);
    }

    {
        // With dummy operators which allocate memory
        bf_list_new(&l, &dummy_ops);
        assert_int_equal(0, l->len);
        assert_null(l->head);
        assert_null(l->tail);

        bf_list_free(&l);
        assert_null(l);

        new_and_fill(&l, 3, &dummy_ops, dummy_filler_head);
        assert_int_equal(3, l->len);
        assert_non_null(l->head);
        assert_non_null(l->tail);

        bf_list_free(&l);
        assert_null(l);
    }
}

Test(list, init_and_clean)
{
    bf_list l;

    {
        // With noop operators
        bf_list_init(&l, &noop_ops);
        assert_int_equal(0, l.len);
        assert_null(l.head);
        assert_null(l.tail);

        bf_list_clean(&l);
        assert_int_equal(0, l.len);
        assert_null(l.head);
        assert_null(l.tail);

        init_and_fill(&l, 3, &noop_ops, bf_list_add_head);
        assert_int_equal(3, l.len);
        assert_non_null(l.head);
        assert_non_null(l.tail);

        bf_list_clean(&l);
        assert_int_equal(0, l.len);
        assert_null(l.head);
        assert_null(l.tail);
    }

    {
        // With dummy operators which allocate memory
        bf_list_init(&l, &dummy_ops);
        assert_int_equal(0, l.len);
        assert_null(l.head);
        assert_null(l.tail);

        bf_list_clean(&l);
        assert_int_equal(0, l.len);
        assert_null(l.head);
        assert_null(l.tail);

        init_and_fill(&l, 3, &dummy_ops, dummy_filler_head);
        assert_int_equal(3, l.len);
        assert_non_null(l.head);
        assert_non_null(l.tail);

        bf_list_clean(&l);
        assert_int_equal(0, l.len);
        assert_null(l.head);
        assert_null(l.tail);
    }
}

Test(list, fill_from_head_and_check)
{
    bf_list list;
    size_t i;

    bf_list_init(&list, &dummy_ops);

    assert_null(bf_list_get_head(&list));

    // Fill list at head with values from 1 to 10, expecting:
    // 10 -> 9 -> ... -> 2 -> 1
    init_and_fill(&list, 10, &dummy_ops, dummy_filler_head);

    // Validate content of the list
    i = bf_list_size(&list);

    bf_list_foreach (&list, it) {
        assert_non_null(it);
        assert_int_equal(i, *(int *)bf_list_node_get_data(it));
        --i;
    }

    i = 1;

    bf_list_foreach_rev (&list, it) {
        assert_non_null(it);
        assert_int_equal(i, *(int *)bf_list_node_get_data(it));
        ++i;
    }

    bf_list_clean(&list);
}

Test(list, iterate_and_remove)
{
    bf_list l;

    init_and_fill(&l, 10, &dummy_ops, dummy_filler_head);

    bf_list_foreach (&l, node)
        bf_list_delete(&l, node);

    assert_int_equal(0, bf_list_size(&l));
    assert_null(l.head);
    assert_null(l.tail);

    bf_list_clean(&l);

    bf_list_foreach_rev (&l, node)
        bf_list_delete(&l, node);

    assert_int_equal(0, bf_list_size(&l));
    assert_null(l.head);
    assert_null(l.tail);

    bf_list_clean(&l);
}

Test(list, fill_from_tail_and_check)
{
    bf_list list;
    size_t i;

    bf_list_init(&list, &dummy_ops);

    assert_null(bf_list_get_head(&list));

    // Fill list at tail with values from 1 to 10, expecting:
    // 1 -> 2 -> ... -> 9 -> 10
    init_and_fill(&list, 10, &dummy_ops, dummy_filler_tail);

    // Validate content of the list
    i = 1;

    bf_list_foreach (&list, it) {
        assert_non_null(it);
        assert_int_equal(i, *(int *)bf_list_node_get_data(it));
        ++i;
    }

    i = bf_list_size(&list);

    bf_list_foreach_rev (&list, it) {
        assert_non_null(it);
        assert_int_equal(i, *(int *)bf_list_node_get_data(it));
        --i;
    }

    bf_list_clean(&list);
}
