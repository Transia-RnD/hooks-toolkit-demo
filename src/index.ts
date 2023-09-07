// NOT EXPORTED
import {
  XrplIntegrationTestContext,
  serverUrl,
  setupClient,
} from '@transia/hooks-toolkit/dist/npm/src/libs/xrpl-helpers'

export async function main(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  // INIT NFO

  await testContext.client.disconnect()
}

main()
