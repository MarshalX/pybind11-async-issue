import asyncio

import async_issue


async def wrap(future):
    on_done_event = asyncio.Event()

    def _done_callback(_):
        on_done_event.set()

    future.add_done_callback(_done_callback)
    await asyncio.wait_for(on_done_event.wait(), timeout=10)

    return future.result()


async def main():
    r = await async_issue.test()
    # works fine
    print('Awaited result of future without thread', r)

    r = await wrap(async_issue.thread_test())
    # works fine (with big delay)
    print('Wrapped future with add_done_callback trick', r)
    r = await async_issue.thread_test()
    # blocked forever. This print doesn't appear
    print('Awaited result of future without wrapping', r)


if __name__ == '__main__':
    loop = asyncio.get_event_loop().run_until_complete(main())
