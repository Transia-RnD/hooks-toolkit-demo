import {
  BaseModel,
  Metadata,
  XFL,
  Currency,
  XRPAddress,
} from '@transia/hooks-toolkit/dist/npm/src/libs/binary-models'

export class ICModel extends BaseModel {
  price: XFL // Price as XFL
  currency: Currency // Currency
  issuer: XRPAddress // Issuer

  constructor(price: XFL, currency: Currency, issuer: XRPAddress) {
    super()
    this.price = price
    this.currency = currency
    this.issuer = issuer
  }

  getMetadata(): Metadata {
    return [
      { field: 'price', type: 'xfl' },
      { field: 'currency', type: 'currency' },
      { field: 'issuer', type: 'xrpAddress' },
    ]
  }

  toJSON() {
    return {
      price: this.price,
      currency: this.currency,
      issuer: this.issuer,
    }
  }
}
